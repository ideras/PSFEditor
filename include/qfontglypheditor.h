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
#define DEFAULT_DOT_WIDTH 24
#define DEFAULT_DOT_HEIGHT 24
#define MIN_DOT_WIDTH  12
#define MIN_DOT_HEIGHT 12

// Canvas information
struct CanvasInfo {
    void setPos(int x, int y) {
        this->x = x;
        this->y = y;
    }
    void setGlyphSize(int w, int h) {
        gw = w;
        gh = h;
    }
    void setDotSize(int w, int h) {
        dw = w;
        dh = h;
    }

    int x1() { return x; }
    int y1() { return y; }
    int x2() { return x + width(); }
    int y2() { return y + height(); }
    int width() { return (dotWidth() * glyphWidth()); }
    int height() { return (dotHeight() * glyphHeight()); }
    int dotWidth() { return dw; }
    int dotHeight() { return dh; }
    int glyphWidth() { return gw; }
    int glyphHeight() { return gh; }

private:
    int x, y;
    int dw, dh;
    int gw, gh;
};

class QFontGlyphEditor : public QWidget
{
    Q_OBJECT
public:
    explicit QFontGlyphEditor(QWidget *parent = nullptr);
    ~QFontGlyphEditor() {}

    void setFont(PSFFont *_font) {
        font = _font;
        updateCanvasInfo();
    }

    void setCurrGlyphIndex(int idx) {
        glyph_index = idx;
        updateCanvasInfo();
        repaint();
    }

    PSFFont *getFont() { return font; }
    PSFGlyph& getCurrGlyph() { return font->getGlyph(static_cast<unsigned>(glyph_index)); }
    int getCurrGlyphIndex() { return  glyph_index; }

    void enableEditor(bool enable) {
        editor_enabled = enable;
    }

    bool hasGlyph() { return ((font != nullptr) && (glyph_index != -1)); }
    bool isGlyphEdited() { return glyph_edited; }

private:
    void updateCanvasInfo();
    void drawGlyph(QPainter &painter);
    bool findPointInCanvas(const QPoint& pt, QPoint &cpt);
    void flipGlyphPoint(const QPoint& gpt);

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);

signals:
    void glyphChanged();

public slots:
private:
    CanvasInfo canvas;
    int dot_width, dot_height;
    QPoint prev_sel_point;
    bool drag_started;
    bool editor_enabled;
    bool glyph_edited;
    PSFFont *font;
    int glyph_index;
};

#endif // QFONTGLYPHEDITOR_H
