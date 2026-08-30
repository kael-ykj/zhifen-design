#include "blockdefinition.h"
#include <QGraphicsItemGroup>
#include <QPainter>

namespace Zhifen {

BlockDefinition::BlockDefinition()
{
    m_created = QDateTime::currentDateTime();
    m_modified = m_created;
}

BlockDefinition::BlockDefinition(const QString &name)
    : m_name(name)
{
    m_created = QDateTime::currentDateTime();
    m_modified = m_created;
}

void BlockDefinition::addItem(QGraphicsItem *item)
{
    if (item && !m_items.contains(item)) {
        m_items.append(item);
        m_modified = QDateTime::currentDateTime();
    }
}

void BlockDefinition::removeItem(QGraphicsItem *item)
{
    m_items.removeAll(item);
    m_modified = QDateTime::currentDateTime();
}

void BlockDefinition::clearItems()
{
    m_items.clear();
    m_modified = QDateTime::currentDateTime();
}

void BlockDefinition::addAttribute(const AttributeDefinition &attr)
{
    // 检查是否已存在
    for (int i = 0; i < m_attributes.size(); i++) {
        if (m_attributes[i].tag == attr.tag) {
            m_attributes[i] = attr;
            m_modified = QDateTime::currentDateTime();
            return;
        }
    }
    m_attributes.append(attr);
    m_modified = QDateTime::currentDateTime();
}

void BlockDefinition::removeAttribute(const QString &tag)
{
    for (int i = 0; i < m_attributes.size(); i++) {
        if (m_attributes[i].tag == tag) {
            m_attributes.removeAt(i);
            m_modified = QDateTime::currentDateTime();
            return;
        }
    }
}

void BlockDefinition::updateAttribute(const QString &tag, const AttributeDefinition &attr)
{
    for (int i = 0; i < m_attributes.size(); i++) {
        if (m_attributes[i].tag == tag) {
            m_attributes[i] = attr;
            m_modified = QDateTime::currentDateTime();
            return;
        }
    }
}

AttributeDefinition* BlockDefinition::attribute(const QString &tag)
{
    for (int i = 0; i < m_attributes.size(); i++) {
        if (m_attributes[i].tag == tag) {
            return &m_attributes[i];
        }
    }
    return nullptr;
}

bool BlockDefinition::hasAttribute(const QString &tag) const
{
    for (const auto &attr : m_attributes) {
        if (attr.tag == tag) return true;
    }
    return false;
}

BlockDefinition* BlockDefinition::clone() const
{
    BlockDefinition *copy = new BlockDefinition(m_name);
    copy->m_basePoint = m_basePoint;
    copy->m_description = m_description;
    copy->m_attributes = m_attributes;
    copy->m_created = m_created;
    copy->m_modified = QDateTime::currentDateTime();
    // 图元深拷贝由调用方处理(需要场景)
    return copy;
}

QRectF BlockDefinition::boundingRect() const
{
    QRectF rect;
    for (auto item : m_items) {
        if (rect.isNull()) {
            rect = item->boundingRect().translated(item->pos());
        } else {
            rect = rect.united(item->boundingRect().translated(item->pos()));
        }
    }
    return rect;
}

} // namespace Zhifen
