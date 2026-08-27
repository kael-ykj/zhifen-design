#include <cmath>
#include "dimensionitem.h"
#include <QPainter>
#include <QtMath>

DimensionItem::DimensionItem(DimType type, const QPointF &p1, const QPointF &p2, const QPointF &dimPos, QGraphicsItem *parent)
    : CadItem(parent), m_type(type), m_p1(p1), m_p2(p2), m_dimPos(dimPos)
{
}

void DimensionItem::setDimension(DimType type, const QPointF &p1, const QPointF &p2, const QPointF &dimPos)
{
    prepareGeometryChange();
    m_type = type;
    m_p1 = p1;
    m_p2 = p2;
    m_dimPos = dimPos;
}

QString DimensionItem::measurementText() const
{
    qreal len = QLineF(m_p1, m_p2).length();
    return QString::number(len, 'f', 2);
}

QRectF DimensionItem::boundingRect() const
{
    QRectF r = QRectF(m_p1, m_p2).normalized();
    r = r.united(QRectF(m_dimPos, m_dimPos));
    qreal padding = m_textHeight * 4;
    return r.adjusted(-padding, -padding, padding, padding);
}

QPainterPath DimensionItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void DimensionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(isSelected() ? QColor(0, 255, 255) : QColor(0, 255, 255));
    painter->setBrush(Qt::NoBrush);

    // 简化绘制：尺寸线+文字
    painter->drawLine(m_p1, m_dimPos);
    painter->drawLine(m_p2, m_dimPos);
    painter->drawLine(m_dimPos, m_dimPos + QPointF(30, 0));

    // 文字
    QFont font;
    font.setPointSizeF(m_textHeight * 2.83);
    painter->setFont(font);
    painter->drawText(m_dimPos + QPointF(15, -5), measurementText());
}

qreal DimensionItem::distanceToPoint(const QPointF &pos) const
{
    QRectF r = boundingRect();
    if (r.contains(pos)) return 0;
    qreal dx = qMax(r.left() - pos.x(), qMax(0.0, pos.x() - r.right()));
    qreal dy = qMax(r.top() - pos.y(), qMax(0.0, pos.y() - r.bottom()));
    return sqrt(dx * dx + dy * dy);
}
