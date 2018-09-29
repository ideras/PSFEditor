#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>
#include <vector>
#include <QMainWindow>
#include <QListWidgetItem>
#include <QFile>
#include "fontglyph.h"
#include "psfutil.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExitApp_triggered();
    void on_actionLoadROMImageFont_triggered();
    void on_listFontGlyphs_itemSelectionChanged();
    void on_actionSave_font_to_ROM_image_triggered();
    void on_actionSave_as_triggered();
    void on_glyphChanged();
    void on_actionCopy_glyph_triggered();
    void on_actionCut_glyph_triggered();
    void on_actionPaste_glyph_triggered();

    void on_actionLoadPSF_triggered();

private:
    void updateGlyphListWidget();
    void updateFileInfo();
    void saveFontToFile();

private:
    QFile currentFile;
    bool fileModified;
    Ui::MainWindow *ui;
    std::vector<FontGlyph> glyphs;
    PSF::SymbInfo symbInfo;
};

#endif // MAINWINDOW_H
