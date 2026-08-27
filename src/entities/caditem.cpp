#include "caditem.h"
#include <QPainter>

CadItem::CadItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

QColor CadItem::effectiveColor() const
{
    if (m_colorByLayer) {
        // 从场景获取图层颜色（简化处理，默认白色）
        return Qt::white;
    }
    return m_color;
}

QPen CadItem::effectivePen() const
{
    QPen pen(effectiveColor());
    pen.setWidthF(m_lineWidth);
    pen.setCosmetic(true); // 线宽不随缩放变化
    if (isSelected()) {
        pen.setColor(QColor(0, 255, 255));
        pen.setWidthF(m_lineWidth + 1);
    }
    return pen;
}
