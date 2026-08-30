#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QKeySequence>
#include <QAction>
#include <QList>

namespace Zhifen {

// 快捷键类别
enum ShortcutCategory {
    SC_File = 0,           // 文件
    SC_Edit = 1,           // 编辑
    SC_View = 2,           // 视图
    SC_Draw = 3,           // 绘图
    SC_Modify = 4,         // 修改
    SC_Dimension = 5,      // 标注
    SC_Layer = 6,          // 图层
    SC_Tools = 7,          // 工具
    SC_Window = 8,         // 窗口
    SC_Help = 9,           // 帮助
    SC_Custom = 10         // 自定义
};

// 快捷键定义
struct ShortcutDef {
    QString id;             // 唯一标识
    QString name;           // 显示名称
    QString description;    // 描述
    QKeySequence defaultKey;// 默认快捷键
    QKeySequence currentKey;// 当前快捷键
    ShortcutCategory category;
    QAction *action = nullptr; // 关联的QAction
    bool editable = true;   // 是否可编辑
};

// 快捷键管理器
class ShortcutManager : public QObject
{
    Q_OBJECT
public:
    static ShortcutManager& instance();

    // 注册快捷键
    void registerShortcut(const QString &id, const QString &name,
                          const QKeySequence &defaultKey,
                          ShortcutCategory category,
                          QAction *action = nullptr);

    // 获取快捷键
    QKeySequence shortcut(const QString &id) const;
    ShortcutDef* shortcutDef(const QString &id);
    QList<ShortcutDef*> allShortcuts() const;
    QList<ShortcutDef*> shortcutsByCategory(ShortcutCategory category) const;

    // 设置快捷键
    bool setShortcut(const QString &id, const QKeySequence &key);
    void resetShortcut(const QString &id);
    void resetAll();

    // 检查冲突
    QString findConflict(const QKeySequence &key, const QString &excludeId = "") const;

    // 类别名称
    QString categoryName(ShortcutCategory category) const;

    // 导出/导入配置
    bool exportConfig(const QString &filePath) const;
    bool importConfig(const QString &filePath);

    // 应用快捷键到所有Action
    void applyAll();

    // 查找快捷键对应的动作
    QAction* actionForShortcut(const QKeySequence &key) const;

signals:
    void shortcutChanged(const QString &id, const QKeySequence &newKey);

private:
    ShortcutManager();
    QMap<QString, ShortcutDef*> m_shortcuts;
    QMap<ShortcutCategory, QString> m_categoryNames;

    void initCategoryNames();
    void loadDefaults();
};

} // namespace Zhifen

#endif // SHORTCUTMANAGER_H
