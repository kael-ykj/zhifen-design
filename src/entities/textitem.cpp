#include <cmath>
#include "textitem.h"
#include <QPainter>
#include <QFontMetricsF>

TextItem::TextItem(const QPointF &pos, const QString &text, qreal height, QGraphicsItem *parent)
    : CadItem(parent), m_text(text), m_height(height)
{
    setPos(pos);
    setFlag(QGraphicsItem::ItemIsMovable, false); // 文字通过位置属性移动
}

QRectF TextItem::boundingRect() const
{
    QFont font;
    font.setPointSizeF(m_height * 2.83); // mm to point
    QFontMetricsF fm(font);
    qreal width = fm.horizontalAdvance(m_text);
    qreal height = fm.height();
    return QRectF(0, -height, width, height).adjusted(-2, -2, 2, 2);
}

QPainterPath TextItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QFont font;
    font.setPointSizeF(m_height * 2.83);
    painter->setFont(font);
    painter->setPen(isSelected() ? QColor(0, 255, 255) : effectiveColor());
    painter->drawText(QPointF(0, 0), m_text);
}

qreal TextItem::distanceToPoint(const QPointF &pos) const
{
    QRectF r = boundingRect();
    r.moveTopLeft(this->pos() + QPointF(0, -r.height()));
    if (r.contains(pos)) return 0;
    qreal dx = qMax(r.left() - pos.x(), qMax(0.0, pos.x() - r.right()));
    qreal dy = qMax(r.top() - pos.y(), qMax(0.0, pos.y() - r.bottom()));
    return sqrt(dx * dx + dy * dy);
}
