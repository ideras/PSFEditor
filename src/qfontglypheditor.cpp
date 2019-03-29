#include <QPainter>
#include <QDebug>
#include "qfontglypheditor.h"

QFontGlyphEditor::QFontGlyphEditor(QWidget *parent) :
    QWidget(parent),
    prev_sel_index(-1),
    drag_started(false),
    editor_enabled(false),
    glyph_edited(false),
    glyph_index(-1)
{ }

QSize QFontGlyphEditor::getDotSize() {
    if (!hasGlyph()) {
        return QSize(DOT_WIDTH, DOT_HEIGHT);
    }
    unsigned gw = font->getWidth();
    unsigned gh = font->getHeight();
    QSize gridSize(gw * DOT_WIDTH, gh * DOT_HEIGHT);
    
    unsigned dotW = DOT_WIDTH;
    unsigned dotH = DOT_HEIGHT;
    
    if (gridSize.width() > size().width()) {
        dotW = size().width() / gw;
    }
    if (gridSize.height() > size().height()) {
        dotH = size().height() / gh;
    }
    return QSize(dotW, dotH);
}

void QFontGlyphEditor::checkPoint(QPoint pt) {
    unsigned gw = font->getWidth();
    unsigned gh = font->getHeight();
    QSize dotSize = getDotSize();
    QSize canvasSize (gw * dotSize.width(), gh * dotSize.height());
    int startX = (size().width() - canvasSize.width()) / 2;
    int startY = (size().height() - canvasSize.height()) / 2;
    int x, y = startY;

    PSFGlyph& glyph = getCurrGlyph();
    for (unsigned gy = 0; gy < gh; gy ++) {
        x = startX;
        for (unsigned gx = 0; gx < gw; gx++) {
            QRect r(x + 1, y + 1, dotSize.width(), dotSize.height());

            if (r.contains(pt)) {
                if (prev_sel_index != (gy  * gw + gx)) {
                    prev_sel_index = gy  * gw + gx;
                    glyph_edited = true;
                    unsigned val = glyph.getPixel(gx, gy);
                    glyph.setPixel(gx, gy, !val);
                    repaint();
                }
                return;
            }
            x += dotSize.width();
        }
        y += dotSize.height();
    }
}

void QFontGlyphEditor::drawGlyph(QPainter &painter) {
    unsigned gw = font->getWidth();
    unsigned gh = font->getHeight();
    QSize dotSize = getDotSize();
    QSize canvasSize (gw * dotSize.width(), gh * dotSize.height());
    int startX = (size().width() - canvasSize.width()) / 2;
    int startY = (size().height() - canvasSize.height()) / 2;
    int x, y = startY;

    const PSFGlyph& glyph = getCurrGlyph();
    for (unsigned gy = 0; gy < gh; gy ++) {
        x = startX;
        for (unsigned gx = 0; gx < gw; gx++) {
            // Glyph dot
            if (glyph.getPixel(gx, gy) != 0) {
                painter.fillRect(x + 1, y + 1, dotSize.width(), dotSize.height(), QColor(255, 140, 0));
            } else {
                painter.fillRect(x + 1, y + 1, dotSize.width(), dotSize.height(), QColor(0, 43, 54));
            }

            // X grid line
            painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
            painter.drawLine(x, startY, x, startY + canvasSize.height());

            x += dotSize.width();
        }

        // Y grid line
        painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
        painter.drawLine(startX, y, startX + canvasSize.width(), y);

        y += dotSize.height();
    }

    painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));
    painter.drawLine(x, startY, x, startY + canvasSize.height());
    painter.drawLine(startX, y, startX + canvasSize.width(), y);
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
    if (e->button() == Qt::LeftButton) {
        if (editor_enabled) {
            prev_sel_index = -1;
            checkPoint(e->pos());
            drag_started = true;
        }
    }
}

void QFontGlyphEditor::mouseMoveEvent(QMouseEvent *e) {
    Q_UNUSED(e);

    if (drag_started) {
        checkPoint(e->pos());
    }
}

void QFontGlyphEditor::mouseReleaseEvent(QMouseEvent *e) {
    Q_UNUSED(e);

    drag_started = false;
}

