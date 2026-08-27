#include <cmath>
#include "rectangleitem.h"
#include <QPainter>
#include <QPainterPathStroker>

RectangleItem::RectangleItem(const QRectF &rect, QGraphicsItem *parent)
    : CadItem(parent), m_rect(rect)
{
}

void RectangleItem::setRectangle(const QRectF &rect)
{
    prepareGeometryChange();
    m_rect = rect;
}

QRectF RectangleItem::boundingRect() const
{
    qreal penWidth = m_lineWidth + 2;
    return m_rect.adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

QPainterPath RectangleItem::shape() const
{
    QPainterPath path;
    path.addRect(m_rect);
    QPainterPathStroker stroker;
    stroker.setWidth(m_lineWidth + 3);
    return stroker.createStroke(path);
}

void RectangleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(effectivePen());
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(m_rect);
}

qreal RectangleItem::distanceToPoint(const QPointF &pos) const
{
    // 简化：到矩形边界的距离
    if (m_rect.contains(pos)) {
        qreal dLeft = pos.x() - m_rect.left();
        qreal dRight = m_rect.right() - pos.x();
        qreal dTop = pos.y() - m_rect.top();
        qreal dBottom = m_rect.bottom() - pos.y();
        return qMin(qMin(dLeft, dRight), qMin(dTop, dBottom));
    }
    qreal dx = qMax(m_rect.left() - pos.x(), qMax(0.0, pos.x() - m_rect.right()));
    qreal dy = qMax(m_rect.top() - pos.y(), qMax(0.0, pos.y() - m_rect.bottom()));
    return sqrt(dx * dx + dy * dy);
}
