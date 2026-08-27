#ifndef DIMENSIONITEM_H
#define DIMENSIONITEM_H

#include "caditem.h"

class DimensionItem : public CadItem
{
public:
    enum DimType { Linear, Aligned, Radius, Diameter, Angular };

    explicit DimensionItem(DimType type = Linear, const QPointF &p1 = QPointF(),
                           const QPointF &p2 = QPointF(), const QPointF &dimPos = QPointF(),
                           QGraphicsItem *parent = nullptr);

    void setDimension(DimType type, const QPointF &p1, const QPointF &p2, const QPointF &dimPos);
    QString measurementText() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "标注"; }
    QPointF center() const override { return m_dimPos; }
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    DimType m_type = Linear;
    QPointF m_p1, m_p2, m_dimPos;
    qreal m_textHeight = 2.5;
    qreal m_arrowSize = 2.5;
};

#endif // DIMENSIONITEM_H
