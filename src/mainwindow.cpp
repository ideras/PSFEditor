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
#include "dlgsymbinfo.h"
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

void MainWindow::updateGlyphListWidget() {
    ui->listFontGlyphs->clear();
    for (unsigned i = 0; i < font.getNumGlyphs(); i++) {
        PSFGlyph& glyph = font.getGlyph(i);

        QString itmText = QString::number(i);
        QListWidgetItem *item = new QListWidgetItem(itmText);
        item->setData(Qt::DisplayRole, QVariant(itmText));
        item->setData(Qt::UserRole, QVariant::fromValue(&glyph));
        ui->listFontGlyphs->addItem(item);
    }
    ui->listFontGlyphs->setCurrentRow(0);
    ui->lblFontTitle->setText(QString("Font Symbols (%1x%2)").arg(font.getWidth()).arg(font.getHeight()));
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

        int index = itmText.toShort();
        ui->widgetGlyphEditor->setCurrGlyphIndex(index);
        ui->widgetGlyphEditor->repaint();
        ui->widgetGlyphEditor->enableEditor(true);

        const PSFGlyph& glyph = font.getGlyph(index);
        const std::vector<unsigned int>& uvals = glyph.getUnicodeValues();

        QString s = "";
        bool first = true;
        for (auto v : uvals) {
            if (first) {
                first = false;
            } else {
                s += " ";
            }
            if (v == 0xFFFE) {
                s += ";";
            } else {
                s += "U+" + QString::number(v, 16);
            }
        }
        QString lblText = "Current symbol <b>" + QString::number(index);
        if (s != "") { lblText += " : " + s; }
        lblText += "</b>";

        ui->lblCurrentSymbolIndex->setText(lblText);
    } else {
        ui->widgetGlyphEditor->enableEditor(false);
        ui->lblCurrentSymbolIndex->setText("");
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

    statusBar()->showMessage(currentFile.fileName());
    setWindowTitle(windowTitle);
}

void MainWindow::on_actionOpenFontFile_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QFileInfo fi(currentFile);
    QString currFilePath = fi.absolutePath();

    if (currFilePath.isEmpty()) {
        currFilePath = QDir::homePath();
    }

    QString filePath = QFileDialog::getOpenFileName(this,
                                 tr("Open Font file"),
                                 currFilePath,
                                 tr("Verilog MIF (*.mif);;PSF file (*.psf)"),
                                 &selectedFilter,
                                 options);
    if (filePath.isEmpty()) {
        return;
    }
    if (selectedFilter.isEmpty()) {
        QMessageBox::information(this, "Error", "Unknown file '" + filePath + "'. Please select a MIF or PSF file");
        return;
    }
    fi.setFile(filePath);
    currentFile.setFileName(fi.absolutePath());

    bool success;
    if (selectedFilter.contains("PSF")) {
        fileType = FileType::PSF;
        success = font.loadFromFile(filePath.toStdString().c_str());
    } else {
        DlgSymbInfo *dlg = new DlgSymbInfo(this);
        if (dlg->exec() != QDialog::Accepted) {
            return;
        }
        int gw = dlg->getWidth();
        int gh = dlg->getHeight();
        dlg->deleteLater();

        fileType = FileType::MIF;
        success = PSF::loadFromVerilogMif(font, gw, gh, filePath.toStdString());
    }

    if (!success) {
        QMessageBox::information(this, "Error", "Error loading file '" + filePath + "'");
        return;
    }
    ui->widgetGlyphEditor->setFont(&font);
    updateGlyphListWidget();
}

void MainWindow::on_actionSaveFont_triggered()
{
    if (currentFile.fileName().isEmpty()) {
        return;
    }
    if (!saveFontToFile()) {
        return;
    }
    QMessageBox::information(this, "Message", "File saved to " + currentFile.fileName());
}

bool MainWindow::saveFontToFile()
{
    QString fileName = currentFile.fileName();

    if (fileType == FileType::MIF) {
        if (!PSF::saveToVerilogMif(font, fileName.toStdString())) {
            QMessageBox::information(nullptr, "Error", "Cannot write file '" + fileName + "'");
            return false;
        }
    } else {
        if (!font.saveToFile(qPrintable(fileName))) {
            QMessageBox::information(nullptr, "Error", "Cannot write file '" + fileName + "'");
            return false;
        }
    }
    fileModified = false;
    updateFileInfo();

    return true;
}

void MainWindow::on_actionExport_VerilogMIF_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;

    if (currentFile.fileName().isEmpty()) {
        return;
    }

    QFileInfo fi(currentFile);
    QString filePath = QFileDialog::getSaveFileName(this,
                                 tr("Save as Verilog MIF"),
                                 fi.path(),
                                 tr("Verilog MIF (*.mif);;All Files (*)"),
                                 &selectedFilter,
                                 options);
    if (filePath.isEmpty()) {
        return;
    }

    fi.setFile(filePath);
    QString fileName = fi.fileName();
    if (!fileName.endsWith(".mif")) {
        fileName += ".mif";
        fi.setFile(fi.dir(), fileName);
        filePath = fi.absolutePath();
    }

    currentFile.setFileName(filePath);
    fileType = FileType::MIF;
    if (!saveFontToFile()) {
        return;
    }
    QMessageBox::information(this, "Message", "File saved to " + filePath);
}

void MainWindow::on_actionExport_PSFFile_triggered()
{
    QFileDialog::Options options;
    QString selectedFilter;

    if (currentFile.fileName().isEmpty()) {
        return;
    }

    QFileInfo fi(currentFile);
    QString filePath = QFileDialog::getSaveFileName(this,
                                 tr("Save as PSF file"),
                                 fi.path(),
                                 tr("PSF files (*.psf);;All Files (*)"),
                                 &selectedFilter,
                                 options);

    if (filePath.isEmpty()) {
        return;
    }

    fi.setFile(filePath);
    QString fileName = fi.fileName();
    if (!fileName.endsWith(".psf")) {
        fileName += ".psf";
        fi.setFile(fi.dir(), fileName);
        filePath = fi.absolutePath();
    }

    currentFile.setFileName(filePath);
    fileType = FileType::PSF;

    if (!saveFontToFile()) {
        return;
    }
    QMessageBox::information(this, "Message", "File saved to " + filePath);
}

void MainWindow::on_actionCopy_glyph_triggered()
{
    if (ui->widgetGlyphEditor->hasGlyph()) {
        const PSFGlyph& glyph = ui->widgetGlyphEditor->getCurrGlyph();

        QClipboard *clipboard = QApplication::clipboard();
        QImage img = PSF::glyphToImage(glyph);
        clipboard->setImage(img);
    }
}

void MainWindow::on_actionCut_glyph_triggered()
{
    if (ui->widgetGlyphEditor->hasGlyph()) {
        PSFGlyph& glyph = ui->widgetGlyphEditor->getCurrGlyph();

        on_actionCopy_glyph_triggered();
        glyph.init(&font);
        ui->widgetGlyphEditor->repaint();
    }
}

void MainWindow::on_actionPaste_glyph_triggered()
{
    if (!ui->widgetGlyphEditor->hasGlyph()) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *clip_data = clipboard->mimeData();
    PSFGlyph& glyph = ui->widgetGlyphEditor->getCurrGlyph();

    if (clip_data->hasText()) {
        QString text = clip_data->text();
        PSF::setGlyphFromText(glyph, text);
        ui->widgetGlyphEditor->repaint();
    } else if (clip_data->hasImage()) {
       QImage img = clipboard->image();
       PSF::setGlyphFromImage(glyph, img);
       ui->widgetGlyphEditor->repaint();
    }
}
