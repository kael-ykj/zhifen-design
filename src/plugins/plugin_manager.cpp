#include "plugin_manager.h"
#include "core_api.h"
#include <QDebug>

namespace Zhifen {

PluginManager::PluginManager() {}
PluginManager::~PluginManager() {
    unloadAll();
}

PluginManager& PluginManager::instance() {
    static PluginManager inst;
    return inst;
}

bool PluginManager::registerPlugin(IPlugin *plugin) {
    if (!plugin) return false;
    if (m_pluginMap.contains(plugin->name())) {
        qWarning() << "插件已存在:" << plugin->name();
        return false;
    }
    m_plugins.append(plugin);
    m_pluginMap[plugin->name()] = plugin;
    qDebug() << "注册插件:" << plugin->name() << plugin->version();
    return true;
}

bool PluginManager::unloadPlugin(const QString &name) {
    if (!m_pluginMap.contains(name)) return false;
    IPlugin *plugin = m_pluginMap[name];
    plugin->unload();
    m_plugins.removeAll(plugin);
    m_pluginMap.remove(name);
    delete plugin;
    qDebug() << "卸载插件:" << name;
    return true;
}

IPlugin* PluginManager::plugin(const QString &name) const {
    return m_pluginMap.value(name, nullptr);
}

QList<PluginMetadata> PluginManager::allMetadata() const {
    QList<PluginMetadata> result;
    for (auto *plugin : m_plugins) {
        PluginMetadata meta;
        meta.name = plugin->name();
        meta.version = plugin->version();
        meta.author = plugin->author();
        meta.description = plugin->description();
        meta.dependencies = plugin->dependencies();
        meta.enabled = plugin->isEnabled();
        meta.loaded = true;
        result.append(meta);
    }
    return result;
}

bool PluginManager::initializeAll(CoreApi *api) {
    m_api = api;
    bool allOk = true;
    for (auto *plugin : m_plugins) {
        if (!plugin->isEnabled()) continue;
        if (!checkDependencies(plugin->name())) {
            qWarning() << "插件依赖不满足:" << plugin->name();
            allOk = false;
            continue;
        }
        if (!plugin->initialize(api)) {
            qWarning() << "插件初始化失败:" << plugin->name();
            allOk = false;
        }
    }
    return allOk;
}

bool PluginManager::executePlugin(const QString &name) {
    IPlugin *plugin = m_pluginMap.value(name, nullptr);
    if (!plugin || !plugin->isEnabled()) return false;
    plugin->execute();
    return true;
}

void PluginManager::unloadAll() {
    for (auto *plugin : m_plugins) {
        plugin->unload();
        delete plugin;
    }
    m_plugins.clear();
    m_pluginMap.clear();
}

bool PluginManager::setEnabled(const QString &name, bool enabled) {
    IPlugin *plugin = m_pluginMap.value(name, nullptr);
    if (!plugin) return false;
    plugin->setEnabled(enabled);
    return true;
}

bool PluginManager::checkDependencies(const QString &name) const {
    IPlugin *plugin = m_pluginMap.value(name, nullptr);
    if (!plugin) return false;
    for (const auto &dep : plugin->dependencies()) {
        if (!m_pluginMap.contains(dep)) {
            qWarning() << "缺少依赖:" << dep << "(插件" << name << "需要)";
            return false;
        }
    }
    return true;
}

} // namespace Zhifen
