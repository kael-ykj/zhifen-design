#ifndef ARCITEM_H
#define ARCITEM_H

#include "caditem.h"

class ArcItem : public CadItem
{
public:
    explicit ArcItem(const QPointF &center = QPointF(), qreal radius = 1.0,
                     qreal startAngle = 0, qreal spanAngle = 180, QGraphicsItem *parent = nullptr);

    void setArc(const QPointF &center, qreal radius, qreal startAngle, qreal spanAngle);
    QPointF centerPoint() const { return m_center; }
    qreal radius() const { return m_radius; }
    qreal startAngle() const { return m_startAngle; }
    qreal spanAngle() const { return m_spanAngle; }
    QPointF startPoint() const;
    QPointF endPoint() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "圆弧"; }
    QPointF center() const override { return m_center; }
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QPointF m_center;
    qreal m_radius = 1.0;
    qreal m_startAngle = 0;  // 度
    qreal m_spanAngle = 180; // 度
};

#endif // ARCITEM_H
