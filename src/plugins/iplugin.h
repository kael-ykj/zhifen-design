#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QString>
#include <QVariantMap>
#include <QStringList>

namespace Zhifen {

// 前向声明
class CoreApi;

// 插件接口
class IPlugin
{
public:
    virtual ~IPlugin() {}

    // 插件元数据
    virtual QString name() const = 0;           // 插件名称
    virtual QString version() const = 0;        // 版本号
    virtual QString author() const = 0;         // 作者
    virtual QString description() const = 0;    // 描述
    virtual QStringList dependencies() const { return QStringList(); } // 依赖插件

    // 生命周期
    virtual bool initialize(CoreApi *api) = 0;  // 初始化
    virtual void execute() = 0;                  // 执行
    virtual void unload() = 0;                   // 卸载

    // 是否启用
    virtual bool isEnabled() const { return m_enabled; }
    virtual void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    bool m_enabled = true;
    CoreApi *m_api = nullptr;
};

// 插件元数据
struct PluginMetadata {
    QString name;
    QString version;
    QString author;
    QString description;
    QStringList dependencies;
    bool enabled = true;
    bool loaded = false;
};

} // namespace Zhifen

#endif // IPLUGIN_H
