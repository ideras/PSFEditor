#include <cstdint>
#include <QString>
#include <QVariant>
#include <QPainter>
#include "qglyphlistwidgetitemdelegate.h"
#include "qfontglypheditor.h"
#include "psfutil.h"

void QGlyphListWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString itm_text = index.data().toString();
    QVariant itmv = index.data(Qt::UserRole);
    PSFGlyph *glyph = itmv.value<PSFGlyph *>();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    int x, y;
    size_t glw = glyph->getFont()->getWidth();
    size_t glh = glyph->getFont()->getHeight();

    int start_x = option.rect.x() + option.fontMetrics.width(itm_text) + 8;
    int start_y = option.rect.y() + (option.rect.height() - glh) / 2;

    QRect r = option.rect;
    r.setX(option.rect.x() + 3);
    painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, itm_text);

    y = start_y;
    for (unsigned gy = 0; gy < glh; gy ++) {
        x = start_x;
        for (unsigned gx = 0; gx < glw; gx++) {
            if (glyph->getPixel(gx, gy) != 0) {
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
    PSFGlyph *glyph = itmv.value<PSFGlyph *>();
    unsigned gh = glyph->getFont()->getHeight();
    int w = option.rect.width();
    int h = qMax(option.fontMetrics.height() + 4, static_cast<int>(gh) + 4);

    return QSize(w, h);
}
