#include "block_manager.h"
#include <QPainter>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QGraphicsItemGroup>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>

namespace Zhifen {

BlockManager::BlockManager() {}
BlockManager::~BlockManager() { clear(); }

BlockManager& BlockManager::instance() {
    static BlockManager inst;
    return inst;
}

QList<QGraphicsItem*> BlockManager::cloneEntities(const QList<QGraphicsItem*> &entities) {
    QList<QGraphicsItem*> clones;
    for (QGraphicsItem *item : entities) {
        if (!item) continue;
        QGraphicsItem *clone = nullptr;

        if (item->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(item);
            QGraphicsLineItem *c = new QGraphicsLineItem(line->line());
            c->setPen(line->pen());
            clone = c;
        } else if (item->type() == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = static_cast<QGraphicsRectItem*>(item);
            QGraphicsRectItem *c = new QGraphicsRectItem(rect->rect());
            c->setPen(rect->pen());
            c->setBrush(rect->brush());
            clone = c;
        } else if (item->type() == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ell = static_cast<QGraphicsEllipseItem*>(item);
            QGraphicsEllipseItem *c = new QGraphicsEllipseItem(ell->rect());
            c->setPen(ell->pen());
            c->setBrush(ell->brush());
            clone = c;
        } else if (item->type() == QGraphicsPolygonItem::Type) {
            QGraphicsPolygonItem *poly = static_cast<QGraphicsPolygonItem*>(item);
            QGraphicsPolygonItem *c = new QGraphicsPolygonItem(poly->polygon());
            c->setPen(poly->pen());
            c->setBrush(poly->brush());
            clone = c;
        } else if (item->type() == QGraphicsPathItem::Type) {
            QGraphicsPathItem *path = static_cast<QGraphicsPathItem*>(item);
            QGraphicsPathItem *c = new QGraphicsPathItem(path->path());
            c->setPen(path->pen());
            c->setBrush(path->brush());
            clone = c;
        } else if (item->type() == QGraphicsTextItem::Type) {
            QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(item);
            QGraphicsTextItem *c = new QGraphicsTextItem(text->toPlainText());
            c->setFont(text->font());
            c->setDefaultTextColor(text->defaultTextColor());
            clone = c;
        }

        if (clone) {
            clone->setPos(item->pos());
            clone->setRotation(item->rotation());
            clone->setScale(item->scale());
            clone->setZValue(item->zValue());
            clone->setVisible(item->isVisible());
            clones.append(clone);
        }
    }
    return clones;
}

QString BlockManager::createBlock(const QString &name, const QPointF &basePoint,
                                    const QList<QGraphicsItem*> &entities,
                                    const QString &description) {
    if (name.isEmpty() || entities.isEmpty()) return QString();
    if (m_blocks.contains(name)) {
        return QString("%1_%2").arg(name).arg(QDateTime::currentMSecsSinceEpoch());
    }

    BlockDefinition *block = new BlockDefinition();
    block->name = name;
    block->description = description;
    block->basePoint = basePoint;
    block->entities = cloneEntities(entities);
    block->created = QDateTime::currentDateTime();
    m_blocks[name] = block;
    return name;
}

bool BlockManager::removeBlock(const QString &name) {
    if (!m_blocks.contains(name)) return false;
    BlockDefinition *block = m_blocks[name];
    for (QGraphicsItem *item : block->entities) {
        delete item;
    }
    block->entities.clear();
    delete block;
    m_blocks.remove(name);
    return true;
}

bool BlockManager::renameBlock(const QString &oldName, const QString &newName) {
    if (!m_blocks.contains(oldName) || m_blocks.contains(newName)) return false;
    BlockDefinition *block = m_blocks[oldName];
    block->name = newName;
    m_blocks.remove(oldName);
    m_blocks[newName] = block;
    return true;
}

bool BlockManager::redefineBlock(const QString &name, const QList<QGraphicsItem*> &entities) {
    if (!m_blocks.contains(name)) return false;
    BlockDefinition *block = m_blocks[name];
    for (QGraphicsItem *item : block->entities) {
        delete item;
    }
    block->entities = cloneEntities(entities);
    return true;
}

BlockDefinition* BlockManager::block(const QString &name) {
    return m_blocks.value(name, nullptr);
}

QList<QString> BlockManager::blockNames() const {
    return m_blocks.keys();
}

QList<BlockDefinition*> BlockManager::allBlocks() const {
    return m_blocks.values();
}

void BlockManager::clear() {
    for (BlockDefinition *block : m_blocks) {
        for (QGraphicsItem *item : block->entities) {
            delete item;
        }
        block->entities.clear();
        delete block;
    }
    m_blocks.clear();
}

bool BlockManager::addAttribute(const QString &blockName, const AttributeDefinition &attr) {
    BlockDefinition *block = m_blocks.value(blockName);
    if (!block) return false;
    for (const auto &a : block->attributes) {
        if (a.tag == attr.tag) return false;
    }
    block->attributes.append(attr);
    return true;
}

bool BlockManager::removeAttribute(const QString &blockName, const QString &tag) {
    BlockDefinition *block = m_blocks.value(blockName);
    if (!block) return false;
    for (int i = 0; i < block->attributes.size(); i++) {
        if (block->attributes[i].tag == tag) {
            block->attributes.removeAt(i);
            return true;
        }
    }
    return false;
}

QList<AttributeDefinition> BlockManager::attributes(const QString &blockName) const {
    BlockDefinition *block = m_blocks.value(blockName);
    if (!block) return QList<AttributeDefinition>();
    return block->attributes;
}

QGraphicsItem* BlockManager::createBlockReference(const QString &blockName, const QPointF &pos,
                                                     qreal scale, qreal rotation,
                                                     const QMap<QString, QString> &attrValues) {
    BlockDefinition *block = m_blocks.value(blockName);
    if (!block) return nullptr;

    QGraphicsItemGroup *group = new QGraphicsItemGroup();
    group->setPos(pos);
    group->setScale(scale);
    group->setRotation(rotation);
    group->setFlag(QGraphicsItem::ItemIsSelectable, true);
    group->setFlag(QGraphicsItem::ItemIsMovable, true);
    group->setData(0, "BLOCK_REF");
    group->setData(1, blockName);

    // 添加块内图元
    for (QGraphicsItem *entity : block->entities) {
        QGraphicsItem *clone = nullptr;
        if (entity->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(entity);
            QGraphicsLineItem *c = new QGraphicsLineItem(line->line(), group);
            c->setPen(line->pen());
            clone = c;
        } else if (entity->type() == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = static_cast<QGraphicsRectItem*>(entity);
            QGraphicsRectItem *c = new QGraphicsRectItem(rect->rect(), group);
            c->setPen(rect->pen());
            c->setBrush(rect->brush());
            clone = c;
        } else if (entity->type() == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ell = static_cast<QGraphicsEllipseItem*>(entity);
            QGraphicsEllipseItem *c = new QGraphicsEllipseItem(ell->rect(), group);
            c->setPen(ell->pen());
            c->setBrush(ell->brush());
            clone = c;
        } else if (entity->type() == QGraphicsTextItem::Type) {
            QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(entity);
            QGraphicsTextItem *c = new QGraphicsTextItem(text->toPlainText(), group);
            c->setFont(text->font());
            c->setDefaultTextColor(text->defaultTextColor());
            clone = c;
        }
        if (clone) {
            clone->setPos(entity->pos() - block->basePoint);
            clone->setRotation(entity->rotation());
            clone->setScale(entity->scale());
        }
    }

    // 添加属性文字
    for (const AttributeDefinition &attr : block->attributes) {
        if (!attr.visible) continue;
        QString value = attrValues.value(attr.tag, attr.defaultValue);
        QGraphicsTextItem *attrText = new QGraphicsTextItem(value, group);
        QFont font("SimSun", attr.textHeight);
        attrText->setFont(font);
        attrText->setPos(attr.position - block->basePoint);
        attrText->setData(0, "BLOCK_ATTR");
        attrText->setData(1, attr.tag);
        attrText->setData(2, value);
    }

    return group;
}

bool BlockManager::saveToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "[BlockLibrary]\n";
    out << "Count=" << m_blocks.size() << "\n";
    for (const QString &name : m_blocks.keys()) {
        BlockDefinition *block = m_blocks[name];
        out << QString("\n[Block:%1]\n").arg(name);
        out << "Name=" << block->name << "\n";
        out << "Description=" << block->description << "\n";
        out << "BaseX=" << block->basePoint.x() << "\n";
        out << "BaseY=" << block->basePoint.y() << "\n";
        out << "EntityCount=" << block->entities.size() << "\n";
        out << "AttrCount=" << block->attributes.size() << "\n";
        for (int i = 0; i < block->attributes.size(); i++) {
            const auto &attr = block->attributes[i];
            out << QString("Attr%1_Tag=%2\n").arg(i).arg(attr.tag);
            out << QString("Attr%1_Prompt=%2\n").arg(i).arg(attr.prompt);
            out << QString("Attr%1_Default=%2\n").arg(i).arg(attr.defaultValue);
        }
    }
    file.close();
    return true;
}

bool BlockManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    clear();
    BlockDefinition *current = nullptr;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("[Block:")) {
            QString name = line.mid(7, line.length() - 8);
            current = new BlockDefinition();
            current->name = name;
            m_blocks[name] = current;
        } else if (current && line.startsWith("Description=")) {
            current->description = line.mid(12);
        } else if (current && line.startsWith("BaseX=")) {
            current->basePoint.setX(line.mid(6).toDouble());
        } else if (current && line.startsWith("BaseY=")) {
            current->basePoint.setY(line.mid(6).toDouble());
        }
    }
    file.close();
    return !m_blocks.isEmpty();
}

} // namespace Zhifen
