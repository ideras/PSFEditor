#include <QPainter>
#include <QDebug>
#include "qfontglypheditor.h"

QFontGlyphEditor::QFontGlyphEditor(QWidget *parent) :
    QWidget(parent),
    dot_width(DEFAULT_DOT_WIDTH),
    dot_height(DEFAULT_DOT_HEIGHT),
    prev_sel_point(0, 0),
    drag_started(false),
    editor_enabled(false),
    glyph_edited(false),
    glyph_index(-1)
{ }

void QFontGlyphEditor::updateCanvasInfo() {
    if (!hasGlyph()) {
        return;
    }
    int dotW = dot_width;
    int dotH = dot_height;

    canvas.setGlyphSize(font->getWidth(), font->getHeight());
    canvas.setDotSize(dotW, dotH);
    if (canvas.width() > size().width()) {
        dotW = size().width() / canvas.glyphWidth();
    }
    if (canvas.height() > size().height()) {
        dotH = size().height() / canvas.glyphHeight();
    }
    canvas.setDotSize(dotW, dotH);
    int x = (size().width() - canvas.width()) / 2;
    int y = (size().height() - canvas.height()) / 2;
    canvas.setPos(x, y);
}

bool QFontGlyphEditor::findPointInCanvas(const QPoint &pt, QPoint &cpt) {
    if (!hasGlyph()) {
        return false;
    }
    int y = canvas.y1();
    for (int gy = 0; gy < canvas.glyphHeight(); gy ++) {
        int x = canvas.x1();
        for (int gx = 0; gx < canvas.glyphWidth(); gx++) {
            QRect r(x + 1, y + 1, canvas.dotWidth(), canvas.dotHeight());

            if (r.contains(pt)) {
                cpt.setX(gx);
                cpt.setY(gy);
                return true;
            }
            x += canvas.dotWidth();
        }
        y += canvas.dotHeight();
    }
    return false;
}

void QFontGlyphEditor::flipGlyphPoint(const QPoint &gpt) {
    PSFGlyph& glyph = getCurrGlyph();
    unsigned gx = static_cast<unsigned>(gpt.x());
    unsigned gy = static_cast<unsigned>(gpt.y());
    glyph.setPixel(gx, gy, !glyph.getPixel(gx, gy));
    repaint();
}

void QFontGlyphEditor::drawGlyph(QPainter &painter) {
    const PSFGlyph& glyph = getCurrGlyph();
    int y = canvas.y1();
    for (int gy = 0; gy < canvas.glyphHeight(); gy ++) {
        int x = canvas.x1();
        for (int gx = 0; gx < canvas.glyphWidth(); gx++) {
            QRect r(x + 1, y + 1, canvas.dotWidth(), canvas.dotHeight());

            // Glyph dot
            if (glyph.getPixel(gx, gy) != 0) {
                painter.fillRect(r, QColor(255, 140, 0));
            } else {
                painter.fillRect(r, QColor(0, 43, 54));
            }

            // X grid line
            painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
            painter.drawLine(x, canvas.y1(), x, canvas.y2());

            x += canvas.dotWidth();
        }

        // Y grid line
        painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
        painter.drawLine(canvas.x1(), y, canvas.x2(), y);

        y += canvas.dotHeight();
    }

    painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
    painter.drawLine(canvas.x2(), canvas.y1(), canvas.x2(), canvas.y2());
    painter.drawLine(canvas.x1(), canvas.y2(), canvas.x2(), canvas.y2());
}

void QFontGlyphEditor::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);

    QPainter painter(this);
    int w = size().width();
    int h = size().height();

    painter.fillRect(0, 0, w, h, QColor(72, 71, 65));

    if (hasGlyph()) {
        drawGlyph(painter);
    }
}

void QFontGlyphEditor::mousePressEvent(QMouseEvent *e) {
    if (!editor_enabled) {
        return;
    }
    if (e->button() == Qt::LeftButton) {
        if (findPointInCanvas(e->pos(), prev_sel_point)) {
            flipGlyphPoint(prev_sel_point);
            drag_started = true;
        }
    }
}

void QFontGlyphEditor::mouseMoveEvent(QMouseEvent *e) {
    Q_UNUSED(e);

    if (drag_started) {
        QPoint cpt;
        if (!findPointInCanvas(e->pos(), cpt)) {
            return;
        }
        if (prev_sel_point != cpt) {
            flipGlyphPoint(cpt);
            prev_sel_point = cpt;
        }
    }
}

void QFontGlyphEditor::mouseReleaseEvent(QMouseEvent *e) {
    Q_UNUSED(e);

    drag_started = false;
}

void QFontGlyphEditor::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);

    updateCanvasInfo();
}

void QFontGlyphEditor::wheelEvent(QWheelEvent *e) {
    if (!hasGlyph()) {
        return;
    }
    if (e->delta() > 0)  {
        dot_width++; dot_height++;
    } else {
        dot_width--; dot_height--;
    }
    if (dot_width < MIN_DOT_WIDTH) {
        dot_width = MIN_DOT_WIDTH;
    }
    if (dot_height < MIN_DOT_HEIGHT) {
        dot_height = MIN_DOT_HEIGHT;
    }
    updateCanvasInfo();
    repaint();
}

