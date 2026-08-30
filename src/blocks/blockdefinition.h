#ifndef BLOCKDEFINITION_H
#define BLOCKDEFINITION_H

#include <QString>
#include <QPointF>
#include <QList>
#include <QMap>
#include <QDateTime>
#include <QColor>
#include <QGraphicsItem>

namespace Zhifen {

// 属性定义
struct AttributeDefinition {
    QString tag;              // 标记(如"型号")
    QString prompt;           // 提示(如"请输入型号")
    QString defaultValue;     // 默认值
    bool visible = true;      // 可见性
    bool constant = false;    // 固定值(不可编辑)
    bool verify = false;      // 验证(插入时二次确认)
    bool preset = false;      // 预设(不提示直接用默认值)
    QPointF position;         // 在块中的位置
    qreal height = 2.5;       // 文字高度
    QColor color = QColor(255, 255, 255);
    int alignment = 0;        // 对齐方式(0=左,1=中,2=右)
    QString layer = "0";      // 图层
    QString textStyle = "Standard"; // 文字样式

    bool operator==(const AttributeDefinition &other) const {
        return tag == other.tag;
    }
};

// 块定义
class BlockDefinition
{
public:
    BlockDefinition();
    explicit BlockDefinition(const QString &name);

    // 基本属性
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; m_modified = QDateTime::currentDateTime(); }
    QPointF basePoint() const { return m_basePoint; }
    void setBasePoint(const QPointF &pt) { m_basePoint = pt; m_modified = QDateTime::currentDateTime(); }
    QString description() const { return m_description; }
    void setDescription(const QString &desc) { m_description = desc; }

    // 图元管理
    void addItem(QGraphicsItem *item);
    void removeItem(QGraphicsItem *item);
    void clearItems();
    QList<QGraphicsItem*> items() const { return m_items; }
    int itemCount() const { return m_items.size(); }

    // 属性定义管理
    void addAttribute(const AttributeDefinition &attr);
    void removeAttribute(const QString &tag);
    void updateAttribute(const QString &tag, const AttributeDefinition &attr);
    AttributeDefinition* attribute(const QString &tag);
    QList<AttributeDefinition> attributes() const { return m_attributes; }
    int attributeCount() const { return m_attributes.size(); }
    bool hasAttribute(const QString &tag) const;

    // 时间戳
    QDateTime created() const { return m_created; }
    QDateTime modified() const { return m_modified; }

    // 预览图(可选)
    void setPreview(const QPixmap &preview) { m_preview = preview; }
    QPixmap preview() const { return m_preview; }

    // 克隆(深拷贝图元)
    BlockDefinition* clone() const;

    // 边界
    QRectF boundingRect() const;

private:
    QString m_name;
    QPointF m_basePoint;
    QList<QGraphicsItem*> m_items;
    QList<AttributeDefinition> m_attributes;
    QString m_description;
    QDateTime m_created;
    QDateTime m_modified;
    QPixmap m_preview;
};

} // namespace Zhifen

#endif // BLOCKDEFINITION_H
