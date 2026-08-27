#include "polylineitem.h"
#include <QPainter>
#include <QPainterPathStroker>

PolylineItem::PolylineItem(const QPolygonF &points, bool closed, QGraphicsItem *parent)
    : CadItem(parent), m_points(points), m_closed(closed)
{
}

void PolylineItem::setPolyline(const QPolygonF &points, bool closed)
{
    prepareGeometryChange();
    m_points = points;
    m_closed = closed;
}

QRectF PolylineItem::boundingRect() const
{
    if (m_points.isEmpty()) return QRectF();
    qreal penWidth = m_lineWidth + 2;
    return m_points.boundingRect().adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

QPainterPath PolylineItem::shape() const
{
    QPainterPath path;
    if (m_points.isEmpty()) return path;
    path.addPolygon(m_points);
    if (m_closed) path.closeSubpath();
    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 3);
    return stroker.createStroke(path);
}

void PolylineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(effectivePen());
    if (m_closed) {
        painter->drawPolygon(m_points);
    } else {
        painter->drawPolyline(m_points);
    }
}

QPointF PolylineItem::center() const
{
    if (m_points.isEmpty()) return QPointF();
    return m_points.boundingRect().center();
}

qreal PolylineItem::distanceToPoint(const QPointF &pos) const
{
    if (m_points.size() < 2) {
        return m_points.isEmpty() ? 1e9 : QLineF(pos, m_points.first()).length();
    }
    qreal minDist = 1e9;
    for (int i = 0; i < m_points.size() - 1; i++) {
        QLineF line(m_points[i], m_points[i + 1]);
        // 点到线段距离
        QPointF v = line.p2() - line.p1();
        QPointF w = pos - line.p1();
        qreal c1 = QPointF::dotProduct(w, v);
        qreal c2 = QPointF::dotProduct(v, v);
        qreal b = c2 > 0 ? c1 / c2 : 0;
        b = qMax(0.0, qMin(1.0, b));
        QPointF pb = line.p1() + b * v;
        qreal d = QLineF(pos, pb).length();
        minDist = qMin(minDist, d);
    }
    return minDist;
}
