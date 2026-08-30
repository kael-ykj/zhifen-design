#ifndef DIMENSIONSTYLEMANAGER_H
#define DIMENSIONSTYLEMANAGER_H

#include "dimension_item.h"
#include <QMap>
#include <QList>
#include <QString>

namespace Zhifen {

// 标注样式管理器
class DimensionStyleManager
{
public:
    static DimensionStyleManager& instance();

    // 添加/删除样式
    void addStyle(const DimensionStyle &style);
    bool removeStyle(const QString &name);
    DimensionStyle* style(const QString &name);
    DimensionStyle* currentStyle();
    void setCurrentStyle(const QString &name);

    // 获取所有样式
    QStringList allStyleNames() const;
    QList<DimensionStyle> allStyles() const;

    // 重命名
    bool renameStyle(const QString &oldName, const QString &newName);

    // 复制样式
    bool copyStyle(const QString &sourceName, const QString &newName);

    // 导入/导出
    bool exportStyles(const QString &filePath) const;
    bool importStyles(const QString &filePath);

    // 重置为默认
    void resetToDefaults();

private:
    DimensionStyleManager();
    QMap<QString, DimensionStyle> m_styles;
    QString m_currentStyle = "Standard";
    void initDefaults();
};

} // namespace Zhifen

#endif // DIMENSIONSTYLEMANAGER_H
