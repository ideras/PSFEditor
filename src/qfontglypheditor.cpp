#include <QPainter>
#include <QDebug>
#include "qfontglypheditor.h"

QFontGlyphEditor::QFontGlyphEditor(QWidget *parent) :
    QWidget(parent),
    prev_sel_index(-1),
    drag_started(false),
    editor_enabled(false),
    glyph_edited(false),
    glyph(nullptr)
{ }

QSize QFontGlyphEditor::getDotSize() {
    if (glyph == nullptr) {
        return QSize(DOT_WIDTH, DOT_HEIGHT);
    }
    QSize glyphSize = glyph->getSize();
    QSize gridSize(glyphSize.width() * DOT_WIDTH, glyphSize.height() * DOT_HEIGHT);
    
    int dotW = DOT_WIDTH;
    int dotH = DOT_HEIGHT;
    
    if (gridSize.width() > size().width()) {
        dotW = size().width() / glyphSize.width();
    }
    if (gridSize.height() > size().height()) {
        dotH = size().height() / glyphSize.height();
    }
    return QSize(dotW, dotH);
}

void QFontGlyphEditor::checkPoint(QPoint pt) {
    QSize glyphSize = glyph->getSize();
    QSize dotSize = getDotSize();
    QSize canvasSize (glyphSize.width()*dotSize.width(), glyphSize.height()*dotSize.height());
    int startX = (size().width() - canvasSize.width()) / 2;
    int startY = (size().height() - canvasSize.height()) / 2;
    int x, y = startY;

    for (int row = 0; row < glyphSize.height(); row ++) {
        x = startX;
        for (int col = 0; col < glyphSize.width(); col++) {
            QRect r(x + 1, y + 1, dotSize.width(), dotSize.height());

            if (r.contains(pt)) {
                if (prev_sel_index != (row  * glyphSize.width() + col)) {
                    prev_sel_index = row  * glyphSize.width() + col;
                    glyph_edited = true;
                    (*glyph)[row][col].flip();
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
    QSize glyphSize = glyph->getSize();
    QSize dotSize = getDotSize();
    QSize canvasSize (glyphSize.width()*dotSize.width(), glyphSize.height()*dotSize.height());
    int startX = (size().width() - canvasSize.width()) / 2;
    int startY = (size().height() - canvasSize.height()) / 2;
    int x, y = startY;

    for (int row = 0; row < glyphSize.height(); row ++) {
        x = startX;
        for (int col = 0; col < glyphSize.width(); col++) {
            // Glyph dot
            if ((*glyph)[row][col]) {
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
    QPainter painter(this);
    int w = size().width();
    int h = size().height();

    painter.fillRect(0, 0, w, h, QColor(72, 71, 65));

    if (glyph != nullptr) {
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
    if (drag_started) {
        checkPoint(e->pos());
    }
}

void QFontGlyphEditor::mouseReleaseEvent(QMouseEvent *e) {
    drag_started = false;
}

/*
void QFontGlyphEditor::setGlyphBitmap(uint8_t *glyph_bmp)
{
    this->glyph_bitmap = glyph_bmp;
    glyph_edited = false;

    if (glyph_bmp != NULL) {
        for (int row = 0; row < GLYPH_HEIGHT; row ++) {
            uint8_t row_bmp = glyph_bmp[row];
            uint8_t mask = 0x80;

            for (int col = 0; col < GLYPH_WIDTH; col++) {
                glyph_points[row * GLYPH_WIDTH + col].value = (row_bmp & mask) != 0;
                mask >>= 1;
            }
        }
    } else {
        for (int row = 0; row < GLYPH_HEIGHT; row ++) {
            for (int col = 0; col < GLYPH_WIDTH; col++) {
                glyph_points[row * GLYPH_WIDTH + col].value = false;
            }
        }
        editor_enabled = false;
    }
    repaint();
}


void QFontGlyphEditor::updateGlyphCanvas()
{
    int w = this->size().width();
    int h = this->size().height();
    int glw = glyph->getWidth();
    int glh = glyph->getHeight();
    int gridRectW = gw * POINT_WIDTH;
    int gridRectH = gh * POINT_HEIGHT;

    int start_x = (w - gridRectW) / 2;
    int start_y = (h - gridRectH) / 2;
    int x, y = start_y;

    releaseData();

    QLine *line;

    glyph_points = new QGlyphPoint[GLYPH_HEIGHT * GLYPH_WIDTH];

    for (int row = 0; row < glh; row ++) {
        x = start_x;
        for (int col = 0; col < glw; col++) {
            QGlyphPoint *gp =  &glyph_points[row * GLYPH_WIDTH + col];

            gp->rect.setRect(x + 1, y + 1, POINT_WIDTH, POINT_HEIGHT);
            gp->value = false;

            QLine *line = new QLine(x, start_y, x, start_y + GLYPH_HEIGHT * POINT_HEIGHT);

            grid_lines.push_back(line);

            x += POINT_WIDTH;
        }

        line = new QLine(start_x, y, start_x + GLYPH_WIDTH * POINT_WIDTH, y);
        grid_lines.push_back(line);

        y += POINT_HEIGHT;
    }
    line = new QLine(start_x, y, start_x + GLYPH_WIDTH * POINT_WIDTH, y);
    grid_lines.push_back(line);
    line = new QLine(x, start_y, x, start_y + GLYPH_HEIGHT * POINT_HEIGHT);
    grid_lines.push_back(line);
}

void QFontGlyphEditor::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    int w = this->size().width();
    int h = this->size().height();

    painter.fillRect(0, 0, w, h, QColor(72, 71, 65));

    if (glyph_points != NULL) {
        for (int row = 0; row < GLYPH_HEIGHT; row++) {
            for (int col = 0; col < GLYPH_WIDTH; col++) {
                QGlyphPoint *gp = &glyph_points[row * GLYPH_WIDTH + col];

                if (gp->value) {
                   painter.setPen(QPen(QColor(255, 140, 0), 1, Qt::SolidLine));
                   painter.setBrush(QBrush(QColor(255, 140, 0)));
                } else {
                   painter.setPen(QPen(QColor(0, 43, 54), 1, Qt::SolidLine));
                   painter.setBrush(QBrush(QColor(0, 43, 54)));
                }

                painter.drawRect(gp->rect);
            }
        }
    }

    // Grid lines
    painter.setPen(QPen(QColor(192, 192, 192), 1, Qt::SolidLine));

    QList<QLine*>::iterator it;

    for (it = grid_lines.begin(); it != grid_lines.end(); it++) {
        QLine *line = *it;
        painter.drawLine(*line);
    }
}

void QFontGlyphEditor::checkPoint(QPoint pt)
{
    int glw = glyph->getWidth();
    int glh = glyph->getHeight();

    for (int row = 0; row < glh; row ++) {
        for (int col = 0; col < glw; col++) {
            int index = row * glw + col;
            QGlyphPoint *gp =  &glyph_points[index];

            if (gp->rect.contains(pt)) {
                if (prev_selected_index != index) {
                    prev_selected_index = index;
                    glyph_edited = true;
                    gp->value = !gp->value;
                    updateGlyphBitmap();
                    repaint();
                }
                return;
            }
        }
    }
}

void QFontGlyphEditor::releaseData()
{
    QList<QLine*>::iterator it = grid_lines.begin();
    while (it != grid_lines.end()) {
        QLine *line = *it;

        delete line;
        it++;
    }
    grid_lines.clear();

    if (glyph_points != nullptr)
        delete [] glyph_points;
}

void QFontGlyphEditor::clearGlyph()
{
    for (int row = 0; row < GLYPH_HEIGHT; row++) {
        for (int col = 0; col < GLYPH_WIDTH; col++) {
            QGlyphPoint *gp = &glyph_points[row * GLYPH_WIDTH + col];

            gp->value = false;
        }
    }
    updateGlyphBitmap();
    repaint();
}

bool QFontGlyphEditor::setGlyphFromText(QString &text) {
    uint8_t *new_glyph_bmp = new uint8_t[GLYPH_HEIGHT];
    int i = 0, index = 0;

    memset(new_glyph_bmp, 0, GLYPH_HEIGHT);

    while (i < text.length()) {
        QString line = "";
        while (i < text.length() && text.at(i) != '\n') {
            line += text.at(i);
            i++;
        }

        bool ok;
        uint8_t val;

        if (line.startsWith("0b")) {
            line = line.mid(2);
            val = line.toInt(&ok, 2);
        }
        else if (line.startsWith("0x")) {
            line = line.mid(2);
            val = line.toInt(&ok, 16);
        }
        else
            ok = false;

        if (!ok) {
            delete [] new_glyph_bmp;
            return false;
        }
        new_glyph_bmp[index++] = val;

        i++;
    }

    memcpy(glyph_bitmap, new_glyph_bmp, GLYPH_HEIGHT);
    for (int row = 0; row < GLYPH_HEIGHT; row ++) {
        uint8_t row_bmp = glyph_bitmap[row];
        uint8_t mask = 0x80;

        for (int col = 0; col < GLYPH_WIDTH; col++) {
            glyph_points[row * GLYPH_WIDTH + col].value = (row_bmp & mask) != 0;
            mask >>= 1;
        }
    }

    repaint();
    delete [] new_glyph_bmp;

    return true;
}

void QFontGlyphEditor::updateGlyphBitmap()
{
    for (int row = 0; row < GLYPH_HEIGHT; row ++) {
        uint8_t mask = 0x80;

        glyph_bitmap[row] = 0;
        for (int col = 0; col < GLYPH_WIDTH; col++) {
            QGlyphPoint *gp = &glyph_points[row * GLYPH_WIDTH + col];

            if (gp->value)
                glyph_bitmap[row] |= mask;

            mask >>= 1;
        }
    }
    emit glyphChanged();
}

*/
