#ifndef QGLYPHLISTWIDGETITEMDELEGATE_H
#define QGLYPHLISTWIDGETITEMDELEGATE_H

#include <QPair>
#include <QStyledItemDelegate>

class QGlyphListWidgetItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QGlyphListWidgetItemDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
};

#endif // QGLYPHLISTWIDGETITEMDELEGATE_H
