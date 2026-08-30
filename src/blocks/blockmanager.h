#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include "blockdefinition.h"

namespace Zhifen {

// 块管理器
class BlockManager : public QObject
{
    Q_OBJECT
public:
    static BlockManager& instance();

    // 块定义管理
    void addBlock(BlockDefinition *block);
    bool removeBlock(const QString &name);
    BlockDefinition* block(const QString &name);
    bool hasBlock(const QString &name) const;
    QStringList allBlockNames() const;
    QList<BlockDefinition*> allBlocks() const;

    // 块重命名
    bool renameBlock(const QString &oldName, const QString &newName);

    // 块重定义(更新块定义，通知所有引用更新)
    void redefineBlock(const QString &name, BlockDefinition *newDef);

    // 块引用计数
    int referenceCount(const QString &name) const;
    void incrementReference(const QString &name);
    void decrementReference(const QString &name);

    // 块库分类
    QStringList categories() const;
    QStringList blocksByCategory(const QString &category) const;
    void setBlockCategory(const QString &blockName, const QString &category);

    // 导入/导出
    bool exportBlock(const QString &name, const QString &filePath) const;
    bool importBlock(const QString &filePath);
    bool exportAllBlocks(const QString &filePath) const;
    bool importBlocks(const QString &filePath);

    // 预置块
    void initDefaultBlocks();

    // 清除
    void clear();

signals:
    void blockAdded(const QString &name);
    void blockRemoved(const QString &name);
    void blockRenamed(const QString &oldName, const QString &newName);
    void blockRedefined(const QString &name);

private:
    BlockManager();
    QMap<QString, BlockDefinition*> m_blocks;
    QMap<QString, int> m_referenceCounts;
    QMap<QString, QString> m_blockCategories; // blockName -> category
};

} // namespace Zhifen

#endif // BLOCKMANAGER_H
