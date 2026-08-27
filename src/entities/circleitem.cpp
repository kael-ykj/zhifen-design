#include "circleitem.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QtMath>

CircleItem::CircleItem(const QPointF &center, qreal radius, QGraphicsItem *parent)
    : CadItem(parent), m_center(center), m_radius(radius)
{
}

void CircleItem::setCircle(const QPointF &center, qreal radius)
{
    prepareGeometryChange();
    m_center = center;
    m_radius = radius;
}

QRectF CircleItem::boundingRect() const
{
    qreal penWidth = m_lineWidth + 2;
    return QRectF(m_center.x() - m_radius - penWidth,
                  m_center.y() - m_radius - penWidth,
                  m_radius * 2 + penWidth * 2,
                  m_radius * 2 + penWidth * 2);
}

QPainterPath CircleItem::shape() const
{
    QPainterPath path;
    path.addEllipse(m_center, m_radius, m_radius);
    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 3);
    return stroker.createStroke(path);
}

void CircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(effectivePen());
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(m_center, m_radius, m_radius);
}

qreal CircleItem::distanceToPoint(const QPointF &pos) const
{
    qreal d = QLineF(pos, m_center).length();
    return qAbs(d - m_radius);
}
