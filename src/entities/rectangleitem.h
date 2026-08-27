#ifndef RECTANGLEITEM_H
#define RECTANGLEITEM_H

#include "caditem.h"

class RectangleItem : public CadItem
{
public:
    explicit RectangleItem(const QRectF &rect = QRectF(), QGraphicsItem *parent = nullptr);

    void setRectangle(const QRectF &rect);
    QRectF rectangle() const { return m_rect; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "矩形"; }
    QPointF center() const override { return m_rect.center(); }
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QRectF m_rect;
};

#endif // RECTANGLEITEM_H
