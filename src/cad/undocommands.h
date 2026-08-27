#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QList>

class CadScene;
class QGraphicsItem;
class CadItem;

// 添加图元命令
class AddItemCommand : public QUndoCommand
{
public:
    AddItemCommand(CadScene *scene, QGraphicsItem *item, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    CadScene *m_scene;
    QGraphicsItem *m_item;
    bool m_ownsItem = false;
};

// 删除图元命令
class RemoveItemsCommand : public QUndoCommand
{
public:
    RemoveItemsCommand(CadScene *scene, const QList<QGraphicsItem*> &items, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    CadScene *m_scene;
    QList<QGraphicsItem*> m_items;
};

// 移动图元命令
class MoveItemsCommand : public QUndoCommand
{
public:
    MoveItemsCommand(CadScene *scene, const QList<QGraphicsItem*> &items, const QPointF &delta, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override { return 1001; }
private:
    CadScene *m_scene;
    QList<QGraphicsItem*> m_items;
    QPointF m_delta;
};

#endif // UNDOCOMMANDS_H
