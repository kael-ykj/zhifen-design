#pragma once

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "mode_control/mode_control_layer.h"
#include "link_calculator.h"

namespace zf {

// 系统图布局配置
struct SystemLayoutConfig {
    double layerSpacing_x{200.0};  // 层间距（水平）
    double nodeSpacing_y{80.0};    // 节点间距（垂直）
    double startX{100.0};
    double startY{100.0};
    bool showPowerLabels{true};    // 显示电平标注
    bool showLossLabels{true};     // 显示损耗标注
    bool showDeviceLabels{true};   // 显示器件编号
};

class SystemDiagramEngine {
public:
    SystemDiagramEngine() = default;

    void setModeManager(ModeManager* mgr) { m_modeMgr = mgr; }
    void setLayoutConfig(const SystemLayoutConfig& cfg) { m_config = cfg; }

    // 从平面图生成系统图
    int generateFromFloor(const Floor* floor, Project* project, SystemDiagram& outDiagram) {
        if (!floor || !project) return ZF_ERR_ARG;

        // 草图模式也允许生成系统图（简易示意版），但不标注详细参数
        bool isSketch = (m_modeMgr && m_modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE);

        outDiagram.diagramId = "SYS_" + floor->floorId;
        outDiagram.floorId = floor->floorId;
        outDiagram.nodes.clear();
        outDiagram.links.clear();

        // 1. 找到信源节点作为起点
        std::vector<const DeviceInstance*> sources;
        for (const auto& dev : floor->devices) {
            auto model = findModel(project, dev.modelId);
            if (model && model->category == DeviceCategory::SIGNAL_SOURCE) {
                sources.push_back(&dev);
            }
        }

        if (sources.empty()) {
            return ZF_ERR_LINK_NO_SOURCE;
        }

        // 2. BFS分层，计算每个器件的层级
        std::map<std::string, int> nodeLayer;
        std::map<std::string, const DeviceInstance*> nodeMap;
        std::queue<std::pair<const DeviceInstance*, int>> bfsQueue;
        std::map<std::string, bool> visited;

        for (const auto* src : sources) {
            bfsQueue.push({src, 0});
            visited[src->instanceId] = true;
            nodeMap[src->instanceId] = src;
        }

        int maxLayer = 0;
        while (!bfsQueue.empty()) {
            auto front = bfsQueue.front();
            bfsQueue.pop();
            const DeviceInstance* dev = front.first;
            int layer = front.second;
            nodeLayer[dev->instanceId] = layer;
            maxLayer = std::max(maxLayer, layer);

            for (const auto& conn : dev->connections) {
                if (!visited[conn.targetInstanceId]) {
                    const DeviceInstance* child = findDevice(floor, conn.targetInstanceId);
                    if (child) {
                        visited[conn.targetInstanceId] = true;
                        nodeMap[conn.targetInstanceId] = child;
                        bfsQueue.push({child, layer + 1});
                    }
                }
            }
        }

        // 3. 按层分组
        std::vector<std::vector<const DeviceInstance*>> layers(maxLayer + 1);
        for (const auto& [id, layer] : nodeLayer) {
            layers[layer].push_back(nodeMap[id]);
        }

        // 4. 布局节点坐标
        for (int layerIdx = 0; layerIdx <= maxLayer; layerIdx++) {
            auto& layerNodes = layers[layerIdx];
            double totalHeight = (layerNodes.size() - 1) * m_config.nodeSpacing_y;
            double startY = m_config.startY - totalHeight / 2.0;

            for (size_t i = 0; i < layerNodes.size(); i++) {
                const auto* dev = layerNodes[i];
                SystemNode node;
                node.nodeId = dev->instanceId;
                node.deviceInstanceId = dev->instanceId;
                node.type = getNodeType(project, dev->modelId);
                node.layoutPos.x = m_config.startX + layerIdx * m_config.layerSpacing_x;
                node.layoutPos.y = startY + i * m_config.nodeSpacing_y;

                // 标注
                auto model = findModel(project, dev->modelId);
                if (model) {
                    node.label = model->displayName;
                    if (!isSketch && m_config.showPowerLabels) {
                        if (!dev->outputPower_dBm.empty()) {
                            auto it = dev->outputPower_dBm.begin();
                            node.label += " (" + std::to_string(it->second).substr(0, 4) + "dBm)";
                        }
                    }
                }

                outDiagram.nodes.push_back(node);
            }
        }

        // 5. 生成连接线
        for (const auto& dev : floor->devices) {
            for (const auto& conn : dev.connections) {
                SystemLink link;
                link.linkId = dev.instanceId + "_" + conn.targetInstanceId;
                link.fromNodeId = dev.instanceId;
                link.toNodeId = conn.targetInstanceId;

                // 查找馈线获取损耗
                for (const auto& cab : floor->cables) {
                    if (cab.segmentId == conn.cableSegmentId) {
                        link.cableModelId = cab.modelId;
                        link.length_m = cab.length_m;
                        // 计算损耗
                        auto cabModel = findModel(project, cab.modelId);
                        if (cabModel) {
                            link.loss_dB = (cab.length_m / 100.0) * cabModel->lossPer100m_900MHz;
                        }
                        break;
                    }
                }

                outDiagram.links.push_back(link);
            }
        }

        return ZF_ERR_OK;
    }

    // 生成系统图文本描述
    std::string generateTextDiagram(const SystemDiagram& diagram) const {
        std::string text;
        text += "========== 系统图 [" + diagram.floorId + "] ==========\n";
        text += "节点数: " + std::to_string(diagram.nodes.size()) + "\n";
        text += "连接数: " + std::to_string(diagram.links.size()) + "\n\n";

        text += "--- 节点布局 ---\n";
        for (const auto& n : diagram.nodes) {
            text += "  [" + n.nodeId + "] " + n.label;
            text += " 位置:(" + std::to_string(n.layoutPos.x) + "," + std::to_string(n.layoutPos.y) + ")";
            text += "\n";
        }

        text += "\n--- 连接关系 ---\n";
        for (const auto& l : diagram.links) {
            text += "  " + l.fromNodeId + " -> " + l.toNodeId;
            text += " 线缆:" + l.cableModelId + " 长度:" + std::to_string(l.length_m) + "m";
            text += " 损耗:" + std::to_string(l.loss_dB).substr(0, 4) + "dB\n";
        }

        return text;
    }

private:
    ModeManager* m_modeMgr{nullptr};
    SystemLayoutConfig m_config;

    std::optional<DeviceModel> findModel(Project* project, const std::string& modelId) {
        for (const auto& m : project->deviceLibrary) {
            if (m.modelId == modelId) return m;
        }
        return std::nullopt;
    }

    const DeviceInstance* findDevice(const Floor* floor, const std::string& instanceId) {
        for (const auto& d : floor->devices) {
            if (d.instanceId == instanceId) return &d;
        }
        return nullptr;
    }

    NodeType getNodeType(Project* project, const std::string& modelId) {
        auto model = findModel(project, modelId);
        if (!model) return NodeType::SOURCE;
        switch (model->category) {
            case DeviceCategory::SIGNAL_SOURCE: return NodeType::SOURCE;
            case DeviceCategory::SPLITTER: return NodeType::SPLITTER;
            case DeviceCategory::COUPLER: return NodeType::COUPLER;
            case DeviceCategory::ANTENNA: return NodeType::ANTENNA;
            default: return NodeType::LOAD;
        }
    }
};

} // namespace zf
