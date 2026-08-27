#include "undocommands.h"
#include "cadscene.h"
#include <QGraphicsItem>

AddItemCommand::AddItemCommand(CadScene *scene, QGraphicsItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene), m_item(item)
{
    setText("添加图元");
}

void AddItemCommand::undo()
{
    if (m_scene && m_item) {
        m_scene->removeItem(m_item);
        m_ownsItem = true;
    }
}

void AddItemCommand::redo()
{
    if (m_scene && m_item) {
        m_scene->addItem(m_item);
        m_ownsItem = false;
    }
}

RemoveItemsCommand::RemoveItemsCommand(CadScene *scene, const QList<QGraphicsItem*> &items, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene), m_items(items)
{
    setText(QString("删除 %1 个图元").arg(items.size()));
}

void RemoveItemsCommand::undo()
{
    if (m_scene) {
        for (auto item : m_items) m_scene->addItem(item);
    }
}

void RemoveItemsCommand::redo()
{
    if (m_scene) {
        for (auto item : m_items) m_scene->removeItem(item);
    }
}

MoveItemsCommand::MoveItemsCommand(CadScene *scene, const QList<QGraphicsItem*> &items, const QPointF &delta, QUndoCommand *parent)
    : QUndoCommand(parent), m_scene(scene), m_items(items), m_delta(delta)
{
    setText("移动图元");
}

void MoveItemsCommand::undo()
{
    for (auto item : m_items) item->moveBy(-m_delta.x(), -m_delta.y());
}

void MoveItemsCommand::redo()
{
    for (auto item : m_items) item->moveBy(m_delta.x(), m_delta.y());
}

bool MoveItemsCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) return false;
    const MoveItemsCommand *cmd = static_cast<const MoveItemsCommand*>(other);
    if (cmd->m_items != m_items) return false;
    m_delta += cmd->m_delta;
    return true;
}
