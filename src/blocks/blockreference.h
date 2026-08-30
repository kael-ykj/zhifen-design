#ifndef BLOCKREFERENCE_H
#define BLOCKREFERENCE_H

#include <QGraphicsItem>
#include <QString>
#include <QPointF>
#include <QMap>
#include <QColor>
#include "blockdefinition.h"

namespace Zhifen {

class BlockManager;

// 块引用(块插入)
class BlockReference : public QGraphicsItem
{
public:
    enum { Type = UserType + 100 };

    explicit BlockReference(const QString &blockName = "", QGraphicsItem *parent = nullptr);
    virtual ~BlockReference();

    // 块名
    QString blockName() const { return m_blockName; }
    void setBlockName(const QString &name);

    // 插入点
    QPointF insertPoint() const { return m_insertPoint; }
    void setInsertPoint(const QPointF &pt) { m_insertPoint = pt; prepareGeometryChange(); update(); }

    // 比例
    qreal scaleX() const { return m_scaleX; }
    qreal scaleY() const { return m_scaleY; }
    void setScaleX(qreal s) { m_scaleX = s; prepareGeometryChange(); update(); }
    void setScaleY(qreal s) { m_scaleY = s; prepareGeometryChange(); update(); }
    void setScale(qreal s) { m_scaleX = s; m_scaleY = s; prepareGeometryChange(); update(); }

    // 旋转
    qreal rotation() const { return m_rotation; }
    void setRotation(qreal angle) { m_rotation = angle; prepareGeometryChange(); update(); }

    // 属性值
    QString attributeValue(const QString &tag) const;
    void setAttributeValue(const QString &tag, const QString &value);
    QMap<QString, QString> attributeValues() const { return m_attributeValues; }
    void setAttributeValues(const QMap<QString, QString> &values) { m_attributeValues = values; update(); }

    // 块定义
    BlockDefinition* blockDefinition() const;

    // 爆炸(分解为独立图元)
    QList<QGraphicsItem*> explode() const;

    // QGraphicsItem接口
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return Type; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QString m_blockName;
    QPointF m_insertPoint;
    qreal m_scaleX = 1.0;
    qreal m_scaleY = 1.0;
    qreal m_rotation = 0.0;
    QMap<QString, QString> m_attributeValues;
    bool m_updating = false;

    void syncAttributesFromDefinition();
    void drawAttribute(QPainter *painter, const AttributeDefinition &attr, const QString &value);
};

} // namespace Zhifen

#endif // BLOCKREFERENCE_H
