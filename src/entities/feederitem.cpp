#include "feederitem.h"
#include <QPainter>
#include <QFontMetricsF>
#include <QtMath>

FeederItem::FeederItem(QGraphicsItem *parent) : CadItem(parent) {}

FeederItem::FeederItem(const QPolygonF &points, FeederType type, QGraphicsItem *parent)
    : CadItem(parent), m_points(points), m_type(type)
{
    updateLength();
}

QString FeederItem::feederTypeName() const
{
    switch (m_type) {
    case Feeder_1_2: return "1/2馈线";
    case Feeder_7_8: return "7/8馈线";
    case Feeder_1_5_8: return "1-5/8馈线";
    case Feeder_5D: return "5D-FB";
    case Feeder_8D: return "8D-FB";
    case Feeder_Fiber: return "光纤";
    case Feeder_Network: return "网线";
    }
    return "馈线";
}

void FeederItem::updateLength()
{
    m_length = 0;
    for (int i = 1; i < m_points.size(); i++) {
        m_length += QLineF(m_points[i-1], m_points[i]).length();
    }
}

QRectF FeederItem::boundingRect() const
{
    if (m_points.isEmpty()) return QRectF();
    QRectF r = m_points.boundingRect();
    qreal pad = m_lineWidth * 2 + 20;
    return r.adjusted(-pad, -pad, pad, pad);
}

QPainterPath FeederItem::shape() const
{
    QPainterPath path;
    if (m_points.size() < 2) return path;
    path.addPolygon(m_points);
    return path;
}

void FeederItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    if (m_points.size() < 2) return;

    QColor c = isSelected() ? QColor(255, 255, 0) : QColor(0, 200, 255);
    painter->setPen(QPen(c, m_lineWidth));
    painter->setBrush(Qt::NoBrush);

    // 绘制馈线路径
    QPolygonF pts = m_points;
    painter->drawPolyline(pts);

    // 绘制连接点小圆
    painter->setBrush(c);
    for (const auto &p : m_points) {
        painter->drawEllipse(p, 2, 2);
    }

    // 在中点绘制标签（类型+长度）
    if (m_points.size() >= 2) {
        int midIdx = m_points.size() / 2;
        QPointF p1 = m_points[midIdx > 0 ? midIdx - 1 : 0];
        QPointF p2 = m_points[midIdx];
        QPointF mid = (p1 + p2) / 2;

        QString label = QString("%1 %2m").arg(feederTypeName()).arg(m_length, 0, 'f', 1);
        QFont font;
        font.setPointSizeF(8);
        painter->setFont(font);
        QFontMetricsF fm(font);
        qreal tw = fm.horizontalAdvance(label);
        qreal th = fm.height();

        // 标签背景
        painter->fillRect(QRectF(mid.x() - tw/2 - 2, mid.y() - th/2 - 1, tw + 4, th + 2),
                          QColor(30, 30, 30, 200));
        painter->setPen(QColor(200, 200, 200));
        painter->drawText(QRectF(mid.x() - tw/2, mid.y() - th/2, tw, th),
                          Qt::AlignCenter, label);
    }
}

QPointF FeederItem::center() const
{
    if (m_points.isEmpty()) return QPointF();
    return m_points.boundingRect().center();
}

qreal FeederItem::distanceToPoint(const QPointF &pos) const
{
    qreal minDist = 1e9;
    for (int i = 1; i < m_points.size(); i++) {
        QLineF line(m_points[i-1], m_points[i]);
        QPointF closest;
        qreal t = ((pos.x() - line.x1()) * (line.x2() - line.x1()) +
                   (pos.y() - line.y1()) * (line.y2() - line.y1())) /
                  (line.dx()*line.dx() + line.dy()*line.dy() + 1e-10);
        t = qMax(0.0, qMin(1.0, t));
        closest = QPointF(line.x1() + t*line.dx(), line.y1() + t*line.dy());
        minDist = qMin(minDist, QLineF(pos, closest).length());
    }
    return minDist;
}
