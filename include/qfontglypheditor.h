#ifndef QFONTGLYPHEDITOR_H
#define QFONTGLYPHEDITOR_H

#include <cstdint>
#include <list>
#include <QWidget>
#include <QMouseEvent>
#include "fontglyph.h"

/*
 * All the glyphs in PSF version 1 are 8x16 pixels
 */
#define GLYPH_WIDTH  8
#define GLYPH_HEIGHT 16

/*
 * One glyph pixel will be zoomed to 24x24 pixels on the editor
 */
#define DOT_WIDTH 24
#define DOT_HEIGHT 24

class QFontGlyphEditor : public QWidget
{
    Q_OBJECT
public:
    explicit QFontGlyphEditor(QWidget *parent = 0);
    ~QFontGlyphEditor() {}

    void setFontGlyph(FontGlyph *glyph) { this->glyph = glyph; repaint(); }
    FontGlyph *getCurrGlyph() { return glyph; }

    void enableEditor(bool enable) {
        editor_enabled = enable;
    }

    bool hasGlyph() { return glyph != nullptr; }
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
    FontGlyph *glyph;
};

#endif // QFONTGLYPHEDITOR_H
