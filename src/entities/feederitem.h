#ifndef FEEDERITEM_H
#define FEEDERITEM_H

#include "caditem.h"
#include <QPolygonF>
#include <QString>

class FeederItem : public CadItem
{
public:
    enum FeederType {
        Feeder_1_2 = 0,    // 1/2馈线
        Feeder_7_8,        // 7/8馈线
        Feeder_1_5_8,      // 1-5/8馈线
        Feeder_5D,         // 5D-FB
        Feeder_8D,         // 8D-FB
        Feeder_Fiber,      // 光纤
        Feeder_Network     // 网线
    };

    explicit FeederItem(QGraphicsItem *parent = nullptr);
    explicit FeederItem(const QPolygonF &points, FeederType type = Feeder_1_2, QGraphicsItem *parent = nullptr);

    void setPoints(const QPolygonF &points) { prepareGeometryChange(); m_points = points; updateLength(); }
    QPolygonF points() const { return m_points; }
    void appendPoint(const QPointF &p) { prepareGeometryChange(); m_points.append(p); updateLength(); }

    void setFeederType(FeederType type) { m_type = type; }
    FeederType feederType() const { return m_type; }
    QString feederTypeName() const;

    qreal length() const { return m_length; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QString entityType() const override { return "馈线"; }
    QPointF center() const override;
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QPolygonF m_points;
    FeederType m_type = Feeder_1_2;
    qreal m_length = 0.0;
    qreal m_lineWidth = 2.0;

    void updateLength();
};

#endif // FEEDERITEM_H
