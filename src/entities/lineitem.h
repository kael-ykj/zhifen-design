#ifndef LINEITEM_H
#define LINEITEM_H

#include "caditem.h"
#include <QLineF>

class LineItem : public CadItem
{
public:
    explicit LineItem(const QPointF &start = QPointF(), const QPointF &end = QPointF(), QGraphicsItem *parent = nullptr);

    void setLine(const QPointF &start, const QPointF &end);
    QLineF line() const { return m_line; }
    QPointF startPoint() const { return m_line.p1(); }
    QPointF endPoint() const { return m_line.p2(); }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "直线"; }
    QPointF center() const override;
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QLineF m_line;
};

#endif // LINEITEM_H
