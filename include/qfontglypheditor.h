#ifndef QFONTGLYPHEDITOR_H
#define QFONTGLYPHEDITOR_H

#include <cstdint>
#include <list>
#include <QWidget>
#include <QMouseEvent>
#include "psf.h"

/*
 * One glyph pixel will be zoomed to 24x24 pixels on the editor
 */
#define DOT_WIDTH 24
#define DOT_HEIGHT 24

class QFontGlyphEditor : public QWidget
{
    Q_OBJECT
public:
    explicit QFontGlyphEditor(QWidget *parent = nullptr);
    ~QFontGlyphEditor() {}

    void setFont(PSFFont *_font) { font = _font; }

    void setCurrGlyphIndex(int idx) {
        glyph_index = idx;
        repaint();
    }

    PSFFont *getFont() { return font; }
    PSFGlyph& getCurrGlyph() { return font->getGlyph(glyph_index); }
    int getCurrGlyphIndex() { return  glyph_index; }

    void enableEditor(bool enable) {
        editor_enabled = enable;
    }

    bool hasGlyph() { return ((font != nullptr) && (glyph_index != -1)); }
    bool isGlyphEdited() { return glyph_edited; }

private:
    QSize getDotSize();
    void drawGlyph(QPainter &painter);
    void checkPoint(QPoint pt);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:
    void glyphChanged();

public slots:
private:
    int prev_sel_index;
    bool drag_started;
    bool editor_enabled;
    bool glyph_edited;
    PSFFont *font;
    int glyph_index;
};

#endif // QFONTGLYPHEDITOR_H
