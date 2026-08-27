#include "lineitem.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QtMath>

LineItem::LineItem(const QPointF &start, const QPointF &end, QGraphicsItem *parent)
    : CadItem(parent), m_line(start, end)
{
}

void LineItem::setLine(const QPointF &start, const QPointF &end)
{
    prepareGeometryChange();
    m_line.setPoints(start, end);
}

QRectF LineItem::boundingRect() const
{
    qreal penWidth = m_lineWidth + 2;
    return QRectF(m_line.p1(), m_line.p2()).normalized()
        .adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

QPainterPath LineItem::shape() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());
    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 3);
    return stroker.createStroke(path);
}

void LineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(effectivePen());
    painter->drawLine(m_line);
}

QPointF LineItem::center() const
{
    return (m_line.p1() + m_line.p2()) / 2;
}

qreal LineItem::distanceToPoint(const QPointF &pos) const
{
    // 点到线段的距离
    QPointF v = m_line.p2() - m_line.p1();
    QPointF w = pos - m_line.p1();
    qreal c1 = QPointF::dotProduct(w, v);
    if (c1 <= 0) return QLineF(pos, m_line.p1()).length();
    qreal c2 = QPointF::dotProduct(v, v);
    if (c2 <= c1) return QLineF(pos, m_line.p2()).length();
    qreal b = c1 / c2;
    QPointF pb = m_line.p1() + b * v;
    return QLineF(pos, pb).length();
}
