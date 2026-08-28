#include <cmath>
#include "dimensionitem.h"
#include <QPainter>
#include <QtMath>
#include <QFontMetricsF>

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
    if (m_type == Radius) {
        qreal r = QLineF(m_p1, m_p2).length();
        return QString("R%1").arg(r, 0, 'f', 2);
    } else if (m_type == Diameter) {
        qreal d = QLineF(m_p1, m_p2).length();
        return QString("Φ%1").arg(d, 0, 'f', 2);
    } else if (m_type == Angular) {
        // p1=圆心, p2=第一边方向点, dimPos=第二边方向点
        QLineF l1(m_p1, m_p2);
        QLineF l2(m_p1, m_dimPos);
        qreal angle = qAbs(l1.angleTo(l2));
        if (angle > 180) angle = 360 - angle;
        return QString("%1°").arg(angle, 0, 'f', 1);
    }
    qreal len = QLineF(m_p1, m_p2).length();
    return QString::number(len, 'f', 2);
}

QRectF DimensionItem::boundingRect() const
{
    QRectF r = QRectF(m_p1, m_p2).normalized();
    r = r.united(QRectF(m_dimPos, m_dimPos));
    qreal padding = m_textHeight * 6;
    return r.adjusted(-padding, -padding, padding, padding);
}

QPainterPath DimensionItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

static void drawArrow(QPainter *painter, const QPointF &tip, qreal angle, qreal size)
{
    QPointF p1(tip.x() - size * qCos(qDegreesToRadians(angle - 30)),
               tip.y() - size * qSin(qDegreesToRadians(angle - 30)));
    QPointF p2(tip.x() - size * qCos(qDegreesToRadians(angle + 30)),
               tip.y() - size * qSin(qDegreesToRadians(angle + 30)));
    QPolygonF arrow;
    arrow << tip << p1 << p2;
    painter->drawPolygon(arrow);
}

void DimensionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    QColor c = isSelected() ? QColor(255, 255, 0) : QColor(0, 255, 255);
    painter->setPen(QPen(c, 0.5));
    painter->setBrush(c);

    QFont font;
    font.setPointSizeF(m_textHeight * 2.83);
    painter->setFont(font);
    QFontMetricsF fm(font);
    QString text = measurementText();
    qreal textW = fm.horizontalAdvance(text);
    qreal textH = fm.height();

    if (m_type == Linear || m_type == Aligned) {
        // 计算尺寸线方向
        QLineF baseLine(m_p1, m_p2);
        qreal angle = baseLine.angle();
        if (m_type == Linear) {
            // 线性标注：水平或垂直，根据dimPos位置判断
            qreal dx = qAbs(m_p2.x() - m_p1.x());
            qreal dy = qAbs(m_p2.y() - m_p1.y());
            if (dx > dy) {
                // 水平标注
                QPointF dimLineStart(m_p1.x(), m_dimPos.y());
                QPointF dimLineEnd(m_p2.x(), m_dimPos.y());
                // 尺寸界线
                painter->drawLine(m_p1, dimLineStart);
                painter->drawLine(m_p2, dimLineEnd);
                // 尺寸线
                painter->drawLine(dimLineStart, dimLineEnd);
                // 箭头
                drawArrow(painter, dimLineStart, 0, m_arrowSize);
                drawArrow(painter, dimLineEnd, 180, m_arrowSize);
                // 文字
                QPointF textPos((dimLineStart.x() + dimLineEnd.x()) / 2 - textW / 2,
                                m_dimPos.y() - m_arrowSize);
                painter->drawText(textPos, text);
            } else {
                // 垂直标注
                QPointF dimLineStart(m_dimPos.x(), m_p1.y());
                QPointF dimLineEnd(m_dimPos.x(), m_p2.y());
                painter->drawLine(m_p1, dimLineStart);
                painter->drawLine(m_p2, dimLineEnd);
                painter->drawLine(dimLineStart, dimLineEnd);
                drawArrow(painter, dimLineStart, 90, m_arrowSize);
                drawArrow(painter, dimLineEnd, 270, m_arrowSize);
                QPointF textPos(m_dimPos.x() + m_arrowSize,
                                (dimLineStart.y() + dimLineEnd.y()) / 2 + textH / 4);
                painter->drawText(textPos, text);
            }
        } else {
            // 对齐标注：尺寸线与两点连线平行
            QLineF dimLine(m_p1, m_p2);
            // 将尺寸线平移到dimPos
            QPointF offset = m_dimPos - QPointF((m_p1.x() + m_p2.x()) / 2, (m_p1.y() + m_p2.y()) / 2);
            QPointF s1 = m_p1 + offset;
            QPointF s2 = m_p2 + offset;
            painter->drawLine(m_p1, s1);
            painter->drawLine(m_p2, s2);
            painter->drawLine(s1, s2);
            drawArrow(painter, s1, angle, m_arrowSize);
            drawArrow(painter, s2, angle + 180, m_arrowSize);
            // 文字旋转
            painter->save();
            QPointF textCenter((s1.x() + s2.x()) / 2, (s1.y() + s2.y()) / 2 - m_arrowSize);
            painter->translate(textCenter);
            painter->rotate(-angle);
            painter->drawText(QPointF(-textW / 2, 0), text);
            painter->restore();
        }
    } else if (m_type == Radius) {
        // p1=圆心, p2=圆弧上点
        painter->drawLine(m_p1, m_p2);
        drawArrow(painter, m_p2, QLineF(m_p1, m_p2).angle(), m_arrowSize);
        QPointF textPos = m_p1 + (m_p2 - m_p1) * 0.5;
        painter->drawText(textPos, text);
    } else if (m_type == Diameter) {
        // p1=圆上一点, p2=对面圆上一点(圆心在中点)
        QPointF center((m_p1.x() + m_p2.x()) / 2, (m_p1.y() + m_p2.y()) / 2);
        painter->drawLine(m_p1, m_p2);
        drawArrow(painter, m_p1, QLineF(center, m_p1).angle(), m_arrowSize);
        drawArrow(painter, m_p2, QLineF(center, m_p2).angle(), m_arrowSize);
        painter->drawText(center + QPointF(m_arrowSize, -m_arrowSize), text);
    } else if (m_type == Angular) {
        // p1=圆心, p2=第一边方向, dimPos=第二边方向
        QLineF l1(m_p1, m_p2);
        QLineF l2(m_p1, m_dimPos);
        qreal a1 = l1.angle();
        qreal a2 = l2.angle();
        qreal radius = qMin(l1.length(), l2.length()) * 0.6;
        // 画圆弧尺寸线
        QRectF arcRect(m_p1.x() - radius, m_p1.y() - radius, radius * 2, radius * 2);
        painter->drawArc(arcRect, (int)(-a1 * 16), (int)((a2 - a1) * 16));
        // 两边延长线
        painter->drawLine(m_p1, m_p2);
        painter->drawLine(m_p1, m_dimPos);
        // 箭头
        qreal midAngle = (a1 + a2) / 2;
        QPointF arcMid(m_p1.x() + radius * qCos(qDegreesToRadians(midAngle)),
                       m_p1.y() - radius * qSin(qDegreesToRadians(midAngle)));
        painter->drawText(arcMid + QPointF(m_arrowSize, -m_arrowSize), text);
    }
}

qreal DimensionItem::distanceToPoint(const QPointF &pos) const
{
    QRectF r = boundingRect();
    if (r.contains(pos)) return 0;
    qreal dx = qMax(r.left() - pos.x(), qMax(0.0, pos.x() - r.right()));
    qreal dy = qMax(r.top() - pos.y(), qMax(0.0, pos.y() - r.bottom()));
    return sqrt(dx * dx + dy * dy);
}
