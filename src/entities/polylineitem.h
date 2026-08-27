#ifndef POLYLINEITEM_H
#define POLYLINEITEM_H

#include "caditem.h"
#include <QPolygonF>

class PolylineItem : public CadItem
{
public:
    explicit PolylineItem(const QPolygonF &points = QPolygonF(), bool closed = false, QGraphicsItem *parent = nullptr);

    void setPolyline(const QPolygonF &points, bool closed = false);
    QPolygonF points() const { return m_points; }
    bool isClosed() const { return m_closed; }
    void setClosed(bool closed) { prepareGeometryChange(); m_closed = closed; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "多段线"; }
    QPointF center() const override;
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QPolygonF m_points;
    bool m_closed = false;
};

#endif // POLYLINEITEM_H
