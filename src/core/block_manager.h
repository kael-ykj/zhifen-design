#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QDateTime>
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace Zhifen {

// 属性定义
struct AttributeDefinition {
    QString tag;           // 属性标签（如"DEVICE_ID"）
    QString prompt;        // 插入提示（如"请输入器件编号"）
    QString defaultValue;  // 默认值
    bool visible = true;   // 是否可见
    bool constant = false; // 是否常量（不可编辑）
    QPointF position;      // 相对位置
    qreal textHeight = 2.5; // 文字高度
};

// 块定义
struct BlockDefinition {
    QString name;          // 块名称
    QString description;   // 说明
    QPointF basePoint;     // 基点
    QList<QGraphicsItem*> entities; // 块内图元（深拷贝）
    QList<AttributeDefinition> attributes; // 属性定义
    QDateTime created;     // 创建时间
    bool isDynamic = false; // 是否动态块
};

// 块属性值
struct AttributeValue {
    QString tag;
    QString value;
    AttributeDefinition definition;
};

// 块管理器
class BlockManager
{
public:
    static BlockManager& instance();
    ~BlockManager();

    // 块定义管理
    QString createBlock(const QString &name, const QPointF &basePoint,
                         const QList<QGraphicsItem*> &entities,
                         const QString &description = "");
    bool removeBlock(const QString &name);
    bool renameBlock(const QString &oldName, const QString &newName);
    bool redefineBlock(const QString &name, const QList<QGraphicsItem*> &entities);
    BlockDefinition* block(const QString &name);
    QList<QString> blockNames() const;
    QList<BlockDefinition*> allBlocks() const;
    int blockCount() const { return m_blocks.size(); }
    void clear();

    // 属性管理
    bool addAttribute(const QString &blockName, const AttributeDefinition &attr);
    bool removeAttribute(const QString &blockName, const QString &tag);
    QList<AttributeDefinition> attributes(const QString &blockName) const;

    // 块引用创建
    QGraphicsItem* createBlockReference(const QString &blockName, const QPointF &pos,
                                          qreal scale = 1.0, qreal rotation = 0.0,
                                          const QMap<QString, QString> &attrValues = QMap<QString, QString>());

    // 保存/加载
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

private:
    BlockManager();
    QMap<QString, BlockDefinition*> m_blocks;

    // 深拷贝图元
    QList<QGraphicsItem*> cloneEntities(const QList<QGraphicsItem*> &entities);
};

} // namespace Zhifen

#endif // BLOCK_MANAGER_H
