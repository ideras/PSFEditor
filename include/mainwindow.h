#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>
#include <vector>
#include <QMainWindow>
#include <QListWidgetItem>
#include <QFile>
#include "psf.h"
#include "psfutil.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExitApp_triggered();
    void on_listFontGlyphs_itemSelectionChanged();
    void on_actionOpenFontFile_triggered();
    void on_actionSaveFont_triggered();
    void on_actionExport_VerilogMIF_triggered();
    void on_actionExport_PSFFile_triggered();
    void on_glyphChanged();
    void on_actionCopy_glyph_triggered();
    void on_actionCut_glyph_triggered();
    void on_actionPaste_glyph_triggered();

private:
    void updateGlyphListWidget();
    void updateFileInfo();
    bool saveFontToFile();

private:
    QString selectedFilter;
    FileType fileType;
    QFile currentFile;
    bool fileModified;
    Ui::MainWindow *ui;
    PSFFont font;
};

#endif // MAINWINDOW_H
