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

namespace zf {

// 单条链路计算结果
struct LinkResult {
    std::string deviceInstanceId;
    std::string deviceModelId;
    std::string portId;
    double inputPower_dBm{0.0};
    double outputPower_dBm{0.0};
    double eirp_dBm{0.0};
    double cumulativeLoss_dB{0.0};
    bool isAntenna{false};
    bool powerOverLimit{false};
    bool lossOverLimit{false};
    std::string remark;
};

// 全项目链路计算报告
struct LinkReport {
    std::vector<LinkResult> results;
    int totalDevices{0};
    int calculatedDevices{0};
    int errorCount{0};
    int warningCount{0};
    double maxAntennaPower_dBm{-100.0};
    double minAntennaPower_dBm{1000.0};
    double avgAntennaPower_dBm{0.0};
    std::vector<std::string> errorMessages;
    std::vector<std::string> warningMessages;
};

class LinkCalculator {
public:
    LinkCalculator() = default;

    void setModeManager(ModeManager* mgr) { m_modeMgr = mgr; }

    // 计算整个项目的链路预算
    // 草图模式下返回限制错误，不执行计算
    int calculateProject(Project* project) {
        if (m_modeMgr && m_modeMgr->checkHeavyComputePermission() != ZF_ERR_OK) {
            return ZF_ERR_SKETCH_MODE_RESTRICTED;
        }
        if (!project) return ZF_ERR_ARG;

        m_report = LinkReport();
        m_report.totalDevices = 0;
        for (const auto& floor : project->floors) {
            m_report.totalDevices += static_cast<int>(floor.devices.size());
        }

        // 遍历每个楼层计算
        for (auto& floor : project->floors) {
            calculateFloor(&floor, project);
        }

        // 统计天线功率
        std::vector<double> antPowers;
        for (const auto& r : m_report.results) {
            if (r.isAntenna) {
                antPowers.push_back(r.eirp_dBm);
                m_report.maxAntennaPower_dBm = std::max(m_report.maxAntennaPower_dBm, r.eirp_dBm);
                m_report.minAntennaPower_dBm = std::min(m_report.minAntennaPower_dBm, r.eirp_dBm);
            }
        }
        if (!antPowers.empty()) {
            double sum = 0;
            for (double p : antPowers) sum += p;
            m_report.avgAntennaPower_dBm = sum / antPowers.size();
        } else {
            m_report.minAntennaPower_dBm = 0.0;
            m_report.maxAntennaPower_dBm = 0.0;
        }

        m_report.errorCount = static_cast<int>(m_report.errorMessages.size());
        m_report.warningCount = static_cast<int>(m_report.warningMessages.size());

        return ZF_ERR_OK;
    }

    // 计算单个楼层
    int calculateFloor(Floor* floor, Project* project) {
        if (!floor || !project) return ZF_ERR_ARG;

        // 找到信源设备
        std::vector<DeviceInstance*> sources;
        for (auto& dev : floor->devices) {
            auto model = findModel(project, dev.modelId);
            if (model && model->category == DeviceCategory::SIGNAL_SOURCE) {
                sources.push_back(&dev);
            }
        }

        if (sources.empty()) {
            m_report.warningMessages.push_back("楼层[" + floor->floorName + "]未找到信源设备");
            return ZF_ERR_LINK_NO_SOURCE;
        }

        // 从每个信源开始BFS遍历
        for (auto* source : sources) {
            // 信源输出功率（默认20dBm或配置值）
            double txPower = 20.0;
            for (const auto& src : project->sources) {
                if (src.deviceModelId == source->modelId) {
                    txPower = src.txPower_dBm;
                    break;
                }
            }
            traverseLink(source, txPower, 0.0, floor, project, "");
        }

        return ZF_ERR_OK;
    }

    const LinkReport& getReport() const { return m_report; }

    // 生成链路预算表文本
    std::string generateReportText() const {
        std::string text;
        text += "========== 链路预算报告 ==========\n";
        text += "总器件数: " + std::to_string(m_report.totalDevices) + "\n";
        text += "已计算器件数: " + std::to_string(m_report.calculatedDevices) + "\n";
        text += "错误数: " + std::to_string(m_report.errorCount) + "\n";
        text += "警告数: " + std::to_string(m_report.warningCount) + "\n";
        if (m_report.maxAntennaPower_dBm > -100) {
            text += "天线最大功率: " + std::to_string(m_report.maxAntennaPower_dBm) + " dBm\n";
            text += "天线最小功率: " + std::to_string(m_report.minAntennaPower_dBm) + " dBm\n";
            text += "天线平均功率: " + std::to_string(m_report.avgAntennaPower_dBm) + " dBm\n";
        }
        text += "\n--- 器件明细 ---\n";
        for (const auto& r : m_report.results) {
            text += "[" + r.deviceInstanceId + "] " + r.deviceModelId;
            text += " 入:" + std::to_string(r.inputPower_dBm) + "dBm";
            text += " 出:" + std::to_string(r.outputPower_dBm) + "dBm";
            if (r.isAntenna) text += " EIRP:" + std::to_string(r.eirp_dBm) + "dBm";
            text += " 累计损耗:" + std::to_string(r.cumulativeLoss_dB) + "dB";
            if (r.powerOverLimit) text += " [功率越限]";
            if (r.lossOverLimit) text += " [损耗过大]";
            text += "\n";
        }
        if (!m_report.errorMessages.empty()) {
            text += "\n--- 错误 ---\n";
            for (const auto& e : m_report.errorMessages) text += "  ERROR: " + e + "\n";
        }
        if (!m_report.warningMessages.empty()) {
            text += "\n--- 警告 ---\n";
            for (const auto& w : m_report.warningMessages) text += "  WARN: " + w + "\n";
        }
        return text;
    }

private:
    ModeManager* m_modeMgr{nullptr};
    LinkReport m_report;
    std::map<std::string, bool> m_visited;

    std::optional<DeviceModel> findModel(Project* project, const std::string& modelId) {
        for (const auto& m : project->deviceLibrary) {
            if (m.modelId == modelId) return m;
        }
        return std::nullopt;
    }

    double getCableLossPer100m(Project* project, const std::string& cableModelId, int freqMHz = 900) {
        for (const auto& m : project->deviceLibrary) {
            if (m.modelId == cableModelId) {
                if (freqMHz >= 1500) return m.lossPer100m_2100MHz;
                return m.lossPer100m_900MHz;
            }
        }
        return 6.0; // 默认1/2馈线损耗
    }

    // 递归遍历链路
    void traverseLink(DeviceInstance* dev, double inputPower, double cumulativeLoss,
                      Floor* floor, Project* project, const std::string& fromPort) {
        if (!dev) return;
        if (m_visited[dev->instanceId]) return;
        m_visited[dev->instanceId] = true;

        auto model = findModel(project, dev->modelId);
        LinkResult result;
        result.deviceInstanceId = dev->instanceId;
        result.deviceModelId = dev->modelId;
        result.inputPower_dBm = inputPower;
        result.cumulativeLoss_dB = cumulativeLoss;

        double outputPower = inputPower;

        if (model) {
            // 根据器件类型计算输出
            switch (model->category) {
                case DeviceCategory::SIGNAL_SOURCE:
                    // 信源：输出即为发射功率
                    outputPower = inputPower;
                    break;

                case DeviceCategory::SPLITTER: {
                    // 功分器：插入损耗 + 均分
                    outputPower = inputPower - model->insertionLoss_dB;
                    // 每个输出端口均分（N路）
                    int outPorts = model->portCount - 1;
                    if (outPorts > 1) {
                        outputPower -= 10.0 * log10(outPorts);
                    }
                    break;
                }

                case DeviceCategory::COUPLER: {
                    // 耦合器：直通端 = 输入 - 直通损耗
                    // 耦合端 = 输入 - 耦合度
                    // 这里简化处理，输出取直通端功率
                    outputPower = inputPower - model->throughLoss_dB;
                    break;
                }

                case DeviceCategory::COMBINER: {
                    // 合路器：插入损耗
                    outputPower = inputPower - model->insertionLoss_dB;
                    break;
                }

                case DeviceCategory::ANTENNA: {
                    // 天线：输入功率 + 增益 = EIRP
                    result.isAntenna = true;
                    outputPower = inputPower;
                    result.eirp_dBm = inputPower + model->gain_dBi;
                    dev->eirp_dBm["default"] = result.eirp_dBm;
                    // 检查功率越限
                    if (result.eirp_dBm > project->maxAntennaPower_dBm) {
                        result.powerOverLimit = true;
                        m_report.errorMessages.push_back(
                            "天线[" + dev->instanceId + "] EIRP " + std::to_string(result.eirp_dBm) +
                            "dBm 超过限值 " + std::to_string(project->maxAntennaPower_dBm) + "dBm");
                    }
                    break;
                }

                default:
                    break;
            }

            dev->inputPower_dBm["default"] = inputPower;
            dev->outputPower_dBm["default"] = outputPower;
            dev->status = LinkStatus::OK;
        }

        result.outputPower_dBm = outputPower;

        // 检查累计损耗
        if (cumulativeLoss > project->maxFeederLoss_dB && result.isAntenna) {
            result.lossOverLimit = true;
            m_report.warningMessages.push_back(
                "器件[" + dev->instanceId + "] 累计损耗 " + std::to_string(cumulativeLoss) +
                "dB 超过建议值 " + std::to_string(project->maxFeederLoss_dB) + "dB");
        }

        m_report.results.push_back(result);
        m_report.calculatedDevices++;

        // 遍历连接的下一级器件
        for (const auto& conn : dev->connections) {
            // 找到目标器件
            DeviceInstance* nextDev = nullptr;
            for (auto& d : floor->devices) {
                if (d.instanceId == conn.targetInstanceId) {
                    nextDev = &d;
                    break;
                }
            }
            if (!nextDev) continue;

            // 计算馈线损耗
            double cableLoss = 0;
            CableSegment* cable = nullptr;
            for (auto& c : floor->cables) {
                if (c.segmentId == conn.cableSegmentId) {
                    cable = &c;
                    break;
                }
            }
            if (cable) {
                double lossPer100m = getCableLossPer100m(project, cable->modelId);
                cableLoss = (cable->length_m / 100.0) * lossPer100m;
            }

            double nextInput = outputPower - cableLoss;
            double nextCumLoss = cumulativeLoss + cableLoss;

            traverseLink(nextDev, nextInput, nextCumLoss, floor, project, conn.toPortId);
        }
    }
};

} // namespace zf
