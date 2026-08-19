#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {

// 插件权限
enum class PluginPermission : int {
    NONE = 0,
    READ_PROJECT = 1 << 0,      // 读取项目数据
    WRITE_PROJECT = 1 << 1,     // 修改项目数据
    RUN_SIMULATION = 1 << 2,    // 运行仿真
    EXPORT_DRAWING = 1 << 3,    // 导出图纸
    ACCESS_FILESYSTEM = 1 << 4, // 文件系统访问
    NETWORK_ACCESS = 1 << 5,    // 网络访问
    ALL = 0xFFFF
};

inline PluginPermission operator|(PluginPermission a, PluginPermission b) {
    return (PluginPermission)((int)a | (int)b);
}
inline bool hasPermission(PluginPermission granted, PluginPermission required) {
    return ((int)granted & (int)required) == (int)required;
}

// 插件元数据
struct PluginMetadata {
    std::string pluginId;
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    PluginPermission permissions{PluginPermission::NONE};
    int apiVersion{1};
    bool enabled{true};
};

// 插件上下文（传递给插件的运行环境）
struct PluginContext {
    Project* project{nullptr};
    Floor* currentFloor{nullptr};
    std::string userId;
    std::map<std::string, std::string> parameters;
};

// 插件执行结果
struct PluginResult {
    bool success{false};
    std::string message;
    std::map<std::string, std::string> outputData;
    int errorCode{0};
};

// 插件接口
class IZfPlugin {
public:
    virtual ~IZfPlugin() = default;
    virtual PluginMetadata getMetadata() const = 0;
    virtual int onInitialize(const PluginContext& ctx) = 0;
    virtual PluginResult onExecute(const PluginContext& ctx) = 0;
    virtual void onShutdown() = 0;
};

// 内置插件示例：器件检查插件
class DeviceCheckPlugin : public IZfPlugin {
public:
    PluginMetadata getMetadata() const override {
        PluginMetadata m;
        m.pluginId = "builtin.device_check";
        m.name = "器件合规性检查";
        m.version = "1.0.0";
        m.author = "智分Design";
        m.description = "检查项目中器件连接完整性和参数合规性";
        m.permissions = PluginPermission::READ_PROJECT;
        return m;
    }
    int onInitialize(const PluginContext&) override { return ZF_ERR_OK; }
    PluginResult onExecute(const PluginContext& ctx) override {
        PluginResult result;
        result.success = true;
        int issues = 0;
        if (ctx.project) {
            for (const auto& floor : ctx.project->floors) {
                for (const auto& dev : floor.devices) {
                    if (dev.connections.empty() && dev.instanceId.find("ANT") != 0) {
                        issues++;
                    }
                }
            }
        }
        result.message = "检查完成，发现 " + std::to_string(issues) + " 个未连接器件";
        result.outputData["issue_count"] = std::to_string(issues);
        return result;
    }
    void onShutdown() override {}
};

// 内置插件示例：覆盖率报告插件
class CoverageReportPlugin : public IZfPlugin {
public:
    PluginMetadata getMetadata() const override {
        PluginMetadata m;
        m.pluginId = "builtin.coverage_report";
        m.name = "覆盖率分析报告";
        m.version = "1.0.0";
        m.author = "智分Design";
        m.description = "生成信号覆盖率分析报告";
        m.permissions = PluginPermission::READ_PROJECT | PluginPermission::RUN_SIMULATION;
        return m;
    }
    int onInitialize(const PluginContext&) override { return ZF_ERR_OK; }
    PluginResult onExecute(const PluginContext& ctx) override {
        PluginResult result;
        result.success = true;
        int deviceCount = 0;
        if (ctx.project) {
            for (const auto& floor : ctx.project->floors) {
                deviceCount += floor.devices.size();
            }
        }
        result.message = "覆盖率报告生成完成，共 " + std::to_string(deviceCount) + " 个器件";
        result.outputData["device_count"] = std::to_string(deviceCount);
        return result;
    }
    void onShutdown() override {}
};

// 插件管理器
class PluginManager {
public:
    PluginManager() {
        // 注册内置插件
        registerPlugin(std::make_shared<DeviceCheckPlugin>());
        registerPlugin(std::make_shared<CoverageReportPlugin>());
    }

    // 注册插件
    int registerPlugin(std::shared_ptr<IZfPlugin> plugin) {
        if (!plugin) return ZF_ERR_ARG;
        auto meta = plugin->getMetadata();
        if (meta.pluginId.empty()) return ZF_ERR_ARG;

        m_plugins[meta.pluginId] = plugin;
        m_metadata[meta.pluginId] = meta;
        return ZF_ERR_OK;
    }

    // 卸载插件
    int unregisterPlugin(const std::string& pluginId) {
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) return ZF_ERR_NOT_FOUND;
        it->second->onShutdown();
        m_plugins.erase(it);
        m_metadata.erase(pluginId);
        return ZF_ERR_OK;
    }

    // 执行插件
    PluginResult executePlugin(const std::string& pluginId,
                               const PluginContext& ctx,
                               PluginPermission grantedPermissions) {
        PluginResult result;
        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) {
            result.success = false;
            result.message = "插件不存在: " + pluginId;
            result.errorCode = ZF_ERR_NOT_FOUND;
            return result;
        }

        auto& meta = m_metadata[pluginId];
        if (!meta.enabled) {
            result.success = false;
            result.message = "插件已禁用: " + pluginId;
            return result;
        }

        // 权限校验
        if (!hasPermission(grantedPermissions, meta.permissions)) {
            result.success = false;
            result.message = "权限不足，插件需要额外权限";
            result.errorCode = ZF_ERR_PERMISSION_DENIED;
            return result;
        }

        // 初始化
        int initResult = it->second->onInitialize(ctx);
        if (initResult != ZF_ERR_OK) {
            result.success = false;
            result.message = "插件初始化失败";
            result.errorCode = initResult;
            return result;
        }

        // 执行
        result = it->second->onExecute(ctx);
        it->second->onShutdown();
        return result;
    }

    // 获取所有插件元数据
    std::vector<PluginMetadata> listPlugins() const {
        std::vector<PluginMetadata> list;
        for (const auto& [id, meta] : m_metadata) {
            list.push_back(meta);
        }
        return list;
    }

    // 启用/禁用插件
    int setPluginEnabled(const std::string& pluginId, bool enabled) {
        auto it = m_metadata.find(pluginId);
        if (it == m_metadata.end()) return ZF_ERR_NOT_FOUND;
        it->second.enabled = enabled;
        return ZF_ERR_OK;
    }

    // 获取插件数量
    int pluginCount() const { return (int)m_plugins.size(); }

    // 检查插件是否存在
    bool hasPlugin(const std::string& pluginId) const {
        return m_plugins.count(pluginId) > 0;
    }

private:
    std::map<std::string, std::shared_ptr<IZfPlugin>> m_plugins;
    std::map<std::string, PluginMetadata> m_metadata;
};

} // namespace zf
