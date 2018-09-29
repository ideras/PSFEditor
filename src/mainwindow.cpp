#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QClipboard>
#include <QMimeData>
#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qglyphlistwidgetitemdelegate.h"
#include "psfutil.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listFontGlyphs->setItemDelegate(new QGlyphListWidgetItemDelegate);
    fileModified = false;
    connect(ui->widgetGlyphEditor, &QFontGlyphEditor::glyphChanged, this, &MainWindow::on_glyphChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionExitApp_triggered() {
    if (fileModified) {
        int res = QMessageBox::question(this, "Confirmation", "File has been changed. Do you want to save it before exit?",
                                        QMessageBox::Yes, QMessageBox::No);

        if (res == QMessageBox::Yes) {
            saveFontToFile();
        }

    }
    qApp->exit();
}

inline int loadGlyph(int& lineIndex, FontGlyph& glyph, QTextStream& in) {
    for (int i = 0; i < 16; i++) {
        if (in.atEnd()) {
            return -1;
        }
        QString line = in.readLine();
        bool ok;
        uint8_t rowPixels = line.toUShort(&ok, 16);
        if (!ok) {
            return -2;
        }

        glyph[i] = rowPixels;
        lineIndex++;
    }
    return 0;
}

void MainWindow::on_actionLoadROMImageFont_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QFileInfo fi(currentFile.fileName());
    QString filePath = fi.absolutePath();

    if (filePath.isEmpty()) {
        filePath = "/home/ideras/classes/Computer_Organization/Project-MIPS32SOC/mif";
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                 tr("Open ROM image font file"),
                                 filePath,
                                 tr("ROM image font file (*.mif);;All Files (*)"),
                                 &selectedFilter,
                                 options);
    if (fileName.isEmpty()) {
        return;
    }

    currentFile.setFileName(fileName);

    if(!currentFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", currentFile.errorString());
        return;
    }

    QTextStream in(&currentFile);

    int line = 0;
    this->ui->listFontGlyphs->clear();
    this->ui->widgetGlyphEditor->setFontGlyph(nullptr);

    glyphs.clear();
    while(!in.atEnd()) {
        FontGlyph glyph(8, 16);
        int res = loadGlyph(line, glyph, in);

        if (res < 0) {
            switch (res) {
            case -1:
                QMessageBox::information(this, "Error reading file", "Too few lines in file");
                break;
            case -2:
                QMessageBox::information(this, "Error reading file", "Invalid hex number at line " + QString::number(line));
                break;
            }
            break;
        }

        glyphs.push_back(std::move(glyph));

        if (glyphs.size() > 256) {
            QMessageBox::information(this, "Error reading file", "Too many lines in file");
            break;
        }
    }
    currentFile.close();

    if (glyphs.size() < 256) {
        QMessageBox::information(this, "Warning", "Too few lines in file, expected 256.");
    } else {
        symbInfo.width = 8;
        symbInfo.height = 16;
        symbInfo.count = 256;
        updateGlyphListWidget();
    }
}

uint32_t glyphRow(uint8_t *&gdata, int rowBytes) {
    uint32_t result;

    switch (rowBytes) {
        case 1:
            result = *gdata++;
            break;
        case 2:
            result = ((uint32_t)(*gdata) << 8) | (uint32_t)(*(gdata + 1));
            gdata += 2;
            break;
        case 3:
            result =  ((uint32_t)(*gdata) << 16)
                    | ((uint32_t)(*(gdata + 1)) << 8)
                    | (uint32_t)(*(gdata + 2));
            gdata += 3;
            break;
        case 4:
            result =  ((uint32_t)(*gdata) << 24)
                    | ((uint32_t)(*(gdata + 1)) << 16)
                    | ((uint32_t)(*(gdata + 2)) << 8)
                    | (uint32_t)(*(gdata + 3));
            gdata += 4;
            break;
    }
    return result;
}

void MainWindow::on_actionLoadPSF_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QFileInfo fi(currentFile.fileName());
    QString filePath = fi.absolutePath();

    if (filePath.isEmpty()) {
        filePath = "/home/ideras/develop/fonts";
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                 tr("Open PSF file"),
                                 filePath,
                                 tr("PSF file (*.psf);;All Files (*)"),
                                 &selectedFilter,
                                 options);
    if (fileName.isEmpty()) {
        return;
    }

    fi.setFile(fileName);
    currentFile.setFileName(fi.absolutePath() + "/" + fi.baseName() + ".mif");

    std::vector<uint8_t> rawFontData;
    int res = PSF::loadFile(fileName.toStdString().c_str(), symbInfo, rawFontData);
    if (res < 0) {
        QMessageBox::information(this, "Error reading file", "Cannot open file '" + fileName + "'");
        return;
    }

    int rowBytes = (symbInfo.width + 7) / 8;
    uint8_t *gd = rawFontData.data();

    glyphs.clear();
    glyphs.reserve(symbInfo.count);
    for (size_t i = 0; i < symbInfo.count; i++) {
        FontGlyph fg(symbInfo.width, symbInfo.height);

        for (size_t ri = 0; ri < symbInfo.height; ri++) {
            uint32_t rd = glyphRow(gd, rowBytes);
            fg[ri] = rd;
        }
        glyphs.push_back(std::move(fg));
    }
    updateGlyphListWidget();
}

void MainWindow::updateGlyphListWidget() {
    ui->listFontGlyphs->clear();
    for (size_t i = 0; i < glyphs.size(); i++) {
        QString itmText = QString::number(i);
        QListWidgetItem *item = new QListWidgetItem(itmText);
        item->setData(Qt::DisplayRole, QVariant(itmText));
        item->setData(Qt::UserRole, QVariant::fromValue(&glyphs[i]));
        ui->listFontGlyphs->addItem(item);
    }
    ui->listFontGlyphs->setCurrentRow(0);
    ui->lblFontTitle->setText(QString("Font Symbols (%1x%2)").arg(symbInfo.width).arg(symbInfo.height));
    ui->listFontGlyphs->repaint();

    fileModified = false;
    updateFileInfo();
}

void MainWindow::on_listFontGlyphs_itemSelectionChanged()
{
    QList<QListWidgetItem*> lstItems = ui->listFontGlyphs->selectedItems();

    if (!lstItems.isEmpty()) {
        QListWidgetItem *selItem = lstItems.at(0);
        QString itmText = selItem->text();

        int index = itmText.toUShort();
        ui->widgetGlyphEditor->setFontGlyph(&glyphs[index]);
        ui->widgetGlyphEditor->repaint();
        ui->widgetGlyphEditor->enableEditor(true);
        ui->lblCurrentSymbolIndex->setText("Current symbol index: " + QString::number(index));
    } else {
        ui->widgetGlyphEditor->enableEditor(false);
        ui->lblCurrentSymbolIndex->setText("");
    }
}

void MainWindow::on_actionSave_font_to_ROM_image_triggered()
{
    if (!currentFile.fileName().isEmpty()) {
        saveFontToFile();
        QMessageBox::information(this, "Message", "File saved to " + currentFile.fileName());
    }
}

void MainWindow::on_actionSave_as_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;

    if (currentFile.fileName().isEmpty())
        return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                 tr("Save font to ROM image"),
                                 "/media/ideras/WINDOWS_D/Development/Verilog-Devel/MIPS32CPU/mips32vgacolor_vtest/",
                                 tr("ROM image font file (*.mif);;All Files (*)"),
                                 &selectedFilter,
                                 options);

    if (!fileName.isEmpty()) {
        currentFile.setFileName(fileName);
        saveFontToFile();
        QMessageBox::information(this, "Message", "File saved to " + fileName);
    }
}

void MainWindow::on_glyphChanged()
{
    fileModified = true;
    updateFileInfo();
}

void MainWindow::updateFileInfo()
{
    QFileInfo fileInfo(currentFile.fileName());
    QString windowTitle = "Fixed Size Font editor - " + fileInfo.fileName();

    if (fileModified) {
        windowTitle += " *";
    }

    this->statusBar()->showMessage(currentFile.fileName());
    this->setWindowTitle(windowTitle);
}

void MainWindow::saveFontToFile()
{
    if(!currentFile.open(QIODevice::WriteOnly)) {
        QMessageBox::information(0, "Error", currentFile.errorString());
        return;
    }
    QTextStream out(&currentFile);

    for (int i = 0; i < 256; i++) {
        out << glyphs[i].toHexString();
    }
    fileModified = false;
    updateFileInfo();

    currentFile.close();
}

void MainWindow::on_actionCopy_glyph_triggered()
{
    FontGlyph *fg = ui->widgetGlyphEditor->getCurrGlyph();

    if (fg != nullptr) {
        QClipboard *clipboard = QApplication::clipboard();
        QImage img = fg->toImage();
        clipboard->setImage(img);
    }
}

void MainWindow::on_actionCut_glyph_triggered()
{
    FontGlyph *fg = ui->widgetGlyphEditor->getCurrGlyph();

    if (fg != nullptr) {
        on_actionCopy_glyph_triggered();
        fg->clear();
        ui->widgetGlyphEditor->repaint();
    }
}

void MainWindow::on_actionPaste_glyph_triggered()
{
    FontGlyph *fg = ui->widgetGlyphEditor->getCurrGlyph();

    if (fg == nullptr) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *clip_data = clipboard->mimeData();

    if (clip_data->hasText()) {
        QString text = clip_data->text();
        fg->updateFromText(text);
        ui->widgetGlyphEditor->repaint();
    } else if (clip_data->hasImage()) {
       QImage img = clipboard->image();
       fg->updateFromImage(img);
       ui->widgetGlyphEditor->repaint();
    }
}
