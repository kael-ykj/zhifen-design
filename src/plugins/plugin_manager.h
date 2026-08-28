#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include "iplugin.h"

namespace Zhifen {

class CoreApi;

// 插件管理器
class PluginManager
{
public:
    static PluginManager& instance();

    // 注册插件
    bool registerPlugin(IPlugin *plugin);

    // 卸载插件
    bool unloadPlugin(const QString &name);

    // 获取插件
    IPlugin* plugin(const QString &name) const;

    // 获取所有插件
    QList<IPlugin*> allPlugins() const { return m_plugins; }

    // 获取插件元数据
    QList<PluginMetadata> allMetadata() const;

    // 初始化所有插件
    bool initializeAll(CoreApi *api);

    // 执行插件
    bool executePlugin(const QString &name);

    // 卸载所有插件
    void unloadAll();

    // 启用/禁用
    bool setEnabled(const QString &name, bool enabled);

    // 检查依赖
    bool checkDependencies(const QString &name) const;

    // 插件数量
    int count() const { return m_plugins.size(); }

private:
    PluginManager();
    ~PluginManager();

    QList<IPlugin*> m_plugins;
    QMap<QString, IPlugin*> m_pluginMap;
    CoreApi *m_api = nullptr;
};

} // namespace Zhifen

#endif // PLUGIN_MANAGER_H
