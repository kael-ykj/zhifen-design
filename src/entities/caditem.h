#ifndef CADITEM_H
#define CADITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QColor>
#include <QRectF>
#include <QPainterPath>

class CadItem : public QGraphicsItem
{
public:
    enum { Type = UserType + 1 };

    explicit CadItem(QGraphicsItem *parent = nullptr);

    // 图层
    QString layer() const { return m_layer; }
    void setLayer(const QString &layer) { m_layer = layer; }

    // 颜色（ByLayer时使用图层颜色）
    QColor color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; }
    bool isColorByLayer() const { return m_colorByLayer; }
    void setColorByLayer(bool byLayer) { m_colorByLayer = byLayer; }

    // 线宽
    qreal lineWidth() const { return m_lineWidth; }
    void setLineWidth(qreal width) { m_lineWidth = width; }

    // 有效颜色（考虑ByLayer）
    virtual QColor effectiveColor() const;

    // 图元类型名称
    virtual QString entityType() const = 0;

    // 几何信息
    virtual QPointF center() const = 0;
    virtual qreal distanceToPoint(const QPointF &pos) const = 0;

    int type() const override { return Type; }

protected:
    QString m_layer = "0";
    QColor m_color = Qt::white;
    bool m_colorByLayer = true;
    qreal m_lineWidth = 0.25;

    QPen effectivePen() const;
};

#endif // CADITEM_H
