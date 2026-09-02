#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include <QIcon>
#include <QString>
#include <QMap>

namespace Zhifen {

// 专业CAD图标生成工厂 - 用QPainter绘制矢量图标
class IconFactory
{
public:
    static IconFactory& instance();
    
    QIcon icon(const QString &name);
    
private:
    IconFactory();
    QMap<QString, QIcon> m_icons;
    
    void initIcons();
    QIcon drawIcon(const QString &name, int size = 32);
};

} // namespace Zhifen

#endif // ICONFACTORY_H
