#ifndef CIRCLEITEM_H
#define CIRCLEITEM_H

#include "caditem.h"

class CircleItem : public CadItem
{
public:
    explicit CircleItem(const QPointF &center = QPointF(), qreal radius = 1.0, QGraphicsItem *parent = nullptr);

    void setCircle(const QPointF &center, qreal radius);
    QPointF centerPoint() const { return m_center; }
    qreal radius() const { return m_radius; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "圆"; }
    QPointF center() const override { return m_center; }
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QPointF m_center;
    qreal m_radius = 1.0;
};

#endif // CIRCLEITEM_H
