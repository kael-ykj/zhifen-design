#include "arcitem.h"
#include <QPainter>
#include <QPainterPathStroker>
#include <QtMath>

ArcItem::ArcItem(const QPointF &center, qreal radius, qreal startAngle, qreal spanAngle, QGraphicsItem *parent)
    : CadItem(parent), m_center(center), m_radius(radius), m_startAngle(startAngle), m_spanAngle(spanAngle)
{
}

void ArcItem::setArc(const QPointF &center, qreal radius, qreal startAngle, qreal spanAngle)
{
    prepareGeometryChange();
    m_center = center;
    m_radius = radius;
    m_startAngle = startAngle;
    m_spanAngle = spanAngle;
}

QPointF ArcItem::startPoint() const
{
    qreal rad = (m_startAngle * 3.14159265358979323846 / 180.0);
    return QPointF(m_center.x() + m_radius * qCos(rad),
                   m_center.y() + m_radius * qSin(rad));
}

QPointF ArcItem::endPoint() const
{
    qreal rad = (m_startAngle + m_spanAngle * 3.14159265358979323846 / 180.0);
    return QPointF(m_center.x() + m_radius * qCos(rad),
                   m_center.y() + m_radius * qSin(rad));
}

QRectF ArcItem::boundingRect() const
{
    qreal penWidth = m_lineWidth + 2;
    return QRectF(m_center.x() - m_radius - penWidth,
                  m_center.y() - m_radius - penWidth,
                  m_radius * 2 + penWidth * 2,
                  m_radius * 2 + penWidth * 2);
}

QPainterPath ArcItem::shape() const
{
    QPainterPath path;
    QRectF rect(m_center.x() - m_radius, m_center.y() - m_radius, m_radius * 2, m_radius * 2);
    path.arcMoveTo(rect, m_startAngle);
    path.arcTo(rect, m_startAngle, m_spanAngle);
    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 3);
    return stroker.createStroke(path);
}

void ArcItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(effectivePen());
    QRectF rect(m_center.x() - m_radius, m_center.y() - m_radius, m_radius * 2, m_radius * 2);
    painter->drawArc(rect, qRound(m_startAngle * 16), qRound(m_spanAngle * 16));
}

qreal ArcItem::distanceToPoint(const QPointF &pos) const
{
    qreal d = QLineF(pos, m_center).length();
    return qAbs(d - m_radius);
}
