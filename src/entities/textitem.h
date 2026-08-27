#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "caditem.h"

class TextItem : public CadItem
{
public:
    explicit TextItem(const QPointF &pos = QPointF(), const QString &text = "", qreal height = 2.5, QGraphicsItem *parent = nullptr);

    void setText(const QString &text) { prepareGeometryChange(); m_text = text; }
    QString text() const { return m_text; }
    void setTextHeight(qreal height) { prepareGeometryChange(); m_height = height; }
    qreal textHeight() const { return m_height; }
    void setPosition(const QPointF &pos) { setPos(pos); }
    QPointF position() const { return pos(); }
    void setRotationAngle(qreal angle) { setRotation(angle); }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString entityType() const override { return "文字"; }
    QPointF center() const override { return pos(); }
    qreal distanceToPoint(const QPointF &pos) const override;

private:
    QString m_text;
    qreal m_height = 2.5;
};

#endif // TEXTITEM_H
