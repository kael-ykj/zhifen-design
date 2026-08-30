#include "blockreference.h"
#include "blockmanager.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/textitem.h"
#include "entities/rectangleitem.h"
#include "entities/arcitem.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QtMath>

namespace Zhifen {

BlockReference::BlockReference(const QString &blockName, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_blockName(blockName)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    syncAttributesFromDefinition();
}

BlockReference::~BlockReference()
{
}

void BlockReference::setBlockName(const QString &name)
{
    m_blockName = name;
    syncAttributesFromDefinition();
    prepareGeometryChange();
    update();
}

QString BlockReference::attributeValue(const QString &tag) const
{
    return m_attributeValues.value(tag, "");
}

void BlockReference::setAttributeValue(const QString &tag, const QString &value)
{
    m_attributeValues[tag] = value;
    update();
}

BlockDefinition* BlockReference::blockDefinition() const
{
    return BlockManager::instance().block(m_blockName);
}

QList<QGraphicsItem*> BlockReference::explode() const
{
    QList<QGraphicsItem*> result;
    BlockDefinition *def = blockDefinition();
    if (!def) return result;

    // 这里只返回定义中的图元，实际深拷贝由场景处理
    // 简化实现：返回空列表，实际爆炸功能在场景中实现
    return result;
}

QRectF BlockReference::boundingRect() const
{
    BlockDefinition *def = blockDefinition();
    if (!def) return QRectF(-5, -5, 10, 10);

    QRectF rect = def->boundingRect();
    if (rect.isNull()) return QRectF(-5, -5, 10, 10);

    // 应用基点偏移
    rect.translate(-def->basePoint());

    // 应用缩放
    rect.setWidth(rect.width() * m_scaleX);
    rect.setHeight(rect.height() * m_scaleY);
    rect.moveLeft(rect.left() * m_scaleX);
    rect.moveTop(rect.top() * m_scaleY);

    // 应用旋转（简化：用外接正方形）
    if (m_rotation != 0) {
        qreal maxDim = qMax(rect.width(), rect.height()) * 1.5;
        QPointF center = rect.center();
        rect = QRectF(center.x() - maxDim, center.y() - maxDim, maxDim * 2, maxDim * 2);
    }

    // 应用插入点偏移
    rect.translate(m_insertPoint);

    // 转换为item本地坐标（因为QGraphicsItem的pos已经是插入点）
    rect.translate(-pos());

    return rect.adjusted(-2, -2, 2, 2);
}

QPainterPath BlockReference::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void BlockReference::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    BlockDefinition *def = blockDefinition();
    if (!def) {
        // 块定义不存在，显示占位符
        painter->setPen(QPen(QColor(255, 0, 0), 0.5, Qt::DashLine));
        painter->drawRect(QRectF(-10, -10, 20, 20));
        painter->setPen(QColor(255, 0, 0));
        painter->drawText(QRectF(-20, 10, 40, 20), Qt::AlignCenter, "块未找到: " + m_blockName);
        return;
    }

    painter->save();
    painter->translate(m_insertPoint);
    painter->rotate(m_rotation * 180 / M_PI);
    painter->scale(m_scaleX, m_scaleY);
    painter->translate(-def->basePoint());

    // 绘制块内所有图元
    for (QGraphicsItem *item : def->items()) {
        painter->save();
        painter->translate(item->pos());

        if (auto line = dynamic_cast<LineItem*>(item)) {
            QPen pen(line->pen());
            pen.setWidthF(0.25);
            painter->setPen(pen);
            painter->drawLine(line->startPoint(), line->endPoint());
        }
        else if (auto circle = dynamic_cast<CircleItem*>(item)) {
            QPen pen(circle->pen());
            pen.setWidthF(0.25);
            painter->setPen(pen);
            painter->setBrush(circle->brush());
            painter->drawEllipse(circle->centerPoint(), circle->radius(), circle->radius());
        }
        else if (auto text = dynamic_cast<TextItem*>(item)) {
            painter->setPen(text->defaultTextColor());
            QFont font = text->font();
            font.setPointSizeF(2.5);
            painter->setFont(font);
            painter->drawText(QPointF(0, 0), text->text());
        }
        else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
            QPen pen(rect->pen());
            pen.setWidthF(0.25);
            painter->setPen(pen);
            painter->setBrush(rect->brush());
            painter->drawRect(rect->rectangle());
        }
        else if (auto arc = dynamic_cast<class ArcItem*>(item)) {
            QPen pen(arc->pen());
            pen.setWidthF(0.25);
            painter->setPen(pen);
            QRectF rect(arc->centerPoint().x() - arc->radius(),
                       arc->centerPoint().y() - arc->radius(),
                       arc->radius() * 2, arc->radius() * 2);
            painter->drawArc(rect, arc->startAngle() * 16, arc->spanAngle() * 16);
        }

        painter->restore();
    }

    // 绘制基点标记
    painter->setPen(QPen(QColor(255, 0, 0), 0.5));
    painter->drawLine(-3, 0, 3, 0);
    painter->drawLine(0, -3, 0, 3);

    // 绘制属性
    for (const auto &attr : def->attributes()) {
        if (attr.visible) {
            QString value = m_attributeValues.value(attr.tag, attr.defaultValue);
            drawAttribute(painter, attr, value);
        }
    }

    painter->restore();

    // 选中状态
    if (isSelected()) {
        painter->setPen(QPen(QColor(0, 120, 255), 0.5, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void BlockReference::drawAttribute(QPainter *painter, const AttributeDefinition &attr, const QString &value)
{
    painter->save();
    painter->setPen(attr.color);
    QFont font(attr.textStyle.isEmpty() ? "SimSun" : attr.textStyle, attr.height);
    painter->setFont(font);

    QPointF pos = attr.position;
    int flags = Qt::AlignLeft;
    if (attr.alignment == 1) flags = Qt::AlignCenter;
    else if (attr.alignment == 2) flags = Qt::AlignRight;

    painter->drawText(QRectF(pos.x(), pos.y() - attr.height, 100, attr.height * 1.5),
                      flags, value);
    painter->restore();
}

void BlockReference::syncAttributesFromDefinition()
{
    BlockDefinition *def = blockDefinition();
    if (!def) return;

    // 确保所有属性定义都有对应的值
    for (const auto &attr : def->attributes()) {
        if (!m_attributeValues.contains(attr.tag)) {
            m_attributeValues[attr.tag] = attr.defaultValue;
        }
    }
}

QVariant BlockReference::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && !m_updating) {
        // 位置变化时更新插入点
        m_insertPoint = pos();
    }
    return QGraphicsItem::itemChange(change, value);
}

} // namespace Zhifen
