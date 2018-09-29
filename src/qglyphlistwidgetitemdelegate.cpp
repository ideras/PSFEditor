#include <cstdint>
#include <QString>
#include <QVariant>
#include <QPainter>
#include "qglyphlistwidgetitemdelegate.h"
#include "qfontglypheditor.h"
#include "fontglyph.h"

void QGlyphListWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString itm_text = index.data().toString();
    QVariant itmv = index.data(Qt::UserRole);
    FontGlyph *fg = itmv.value<FontGlyph *>();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    int x, y;
    size_t glw = fg->getWidth();
    size_t glh = fg->getHeight();

    int start_x = option.rect.x() + option.fontMetrics.width(itm_text) + 8;
    int start_y = option.rect.y() + (option.rect.height() - glh) / 2;

    QRect r = option.rect;
    r.setX(option.rect.x() + 3);
    painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, itm_text);

    y = start_y;
    for (size_t row = 0; row < glh; row ++) {
        x = start_x;
        for (size_t col = 0; col < glw; col++) {
            if ((*fg)[row][col]) {
                painter->drawPoint(x, y);
            }
            x++;
        }
        y++;
    }
}

QSize QGlyphListWidgetItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant itmv = index.data(Qt::UserRole);
    FontGlyph *fg = itmv.value<FontGlyph *>();
    int w = option.rect.width();
    int h = qMax(option.fontMetrics.height() + 4, static_cast<int>(fg->getHeight()) + 4);

    return QSize(w, h);
}
