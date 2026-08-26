#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "engine/cost_estimator.h"
#include "engine/link_calculator.h"

namespace zf {

// 报告配置
struct ReportConfig {
    std::string companyName{"智分Design"};
    std::string designer{"系统自动生成"};
    std::string reviewer{""};
    bool includeDeviceList{true};
    bool includeCostEstimate{true};
    bool includeLinkBudget{true};
    bool includeCoverageStats{true};
};

// 设计报告生成引擎
class ReportGenerator {
public:
    ReportGenerator() = default;

    void setConfig(const ReportConfig& cfg) { m_config = cfg; }

    // 生成HTML报告
    int generateHtmlReport(const Project* project,
                           const LinkReport* linkReport,
                           const CostSummary* costSummary,
                           std::string& outHtml) {
        if (!project) return ZF_ERR_ARG;

        std::ostringstream html;
        html << std::fixed << std::setprecision(2);

        // HTML头部
        html << "<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n";
        html << "<meta charset=\"UTF-8\">\n";
        html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        html << "<title>" << project->projectName << " - 室内分布系统设计报告</title>\n";
        html << "<style>\n";
        html << "body { font-family: 'Microsoft YaHei', Arial, sans-serif; margin: 40px; color: #333; line-height: 1.6; }\n";
        html << "h1 { color: #1a5276; border-bottom: 3px solid #1a5276; padding-bottom: 10px; }\n";
        html << "h2 { color: #2874a6; margin-top: 30px; border-left: 4px solid #2874a6; padding-left: 10px; }\n";
        html << "h3 { color: #2e86c1; }\n";
        html << "table { border-collapse: collapse; width: 100%; margin: 15px 0; }\n";
        html << "th, td { border: 1px solid #bdc3c7; padding: 8px 12px; text-align: left; }\n";
        html << "th { background-color: #2874a6; color: white; }\n";
        html << "tr:nth-child(even) { background-color: #f2f3f4; }\n";
        html << ".summary-box { background: #eaf2f8; border: 1px solid #2874a6; border-radius: 8px; padding: 20px; margin: 20px 0; }\n";
        html << ".cost-total { font-size: 1.4em; font-weight: bold; color: #c0392b; }\n";
        html << ".metric { display: inline-block; margin: 10px 20px 10px 0; }\n";
        html << ".metric-value { font-size: 1.3em; font-weight: bold; color: #1a5276; }\n";
        html << ".metric-label { font-size: 0.9em; color: #7f8c8d; }\n";
        html << ".footer { margin-top: 50px; padding-top: 20px; border-top: 1px solid #bdc3c7; color: #7f8c8d; font-size: 0.9em; }\n";
        html << ".warning { color: #e67e22; }\n";
        html << ".error { color: #c0392b; }\n";
        html << ".success { color: #27ae60; }\n";
        html << "</style>\n</head>\n<body>\n";

        // 标题
        html << "<h1>" << project->projectName << "</h1>\n";
        html << "<p><strong>室内分布系统设计报告</strong></p>\n";

        // 工程信息
        html << "<h2>一、工程信息</h2>\n";
        html << "<div class=\"summary-box\">\n";
        html << "<table>\n";
        html << "<tr><th>项目名称</th><td>" << project->projectName << "</td></tr>\n";
        html << "<tr><th>项目编号</th><td>" << project->projectId << "</td></tr>\n";
        html << "<tr><th>设计单位</th><td>" << m_config.companyName << "</td></tr>\n";
        html << "<tr><th>设计人</th><td>" << m_config.designer << "</td></tr>\n";
        html << "<tr><th>楼层数</th><td>" << project->floors.size() << " 层</td></tr>\n";
        int totalDevices = 0;
        double totalCable = 0;
        for (const auto& f : project->floors) {
            totalDevices += f.devices.size();
            for (const auto& c : f.cables) totalCable += c.length_m;
        }
        html << "<tr><th>器件总数</th><td>" << totalDevices << " 个</td></tr>\n";
        html << "<tr><th>线缆总长</th><td>" << totalCable << " 米</td></tr>\n";
        html << "</table>\n</div>\n";

        // 楼层明细
        html << "<h2>二、楼层明细</h2>\n";
        html << "<table>\n<tr><th>楼层</th><th>楼层名称</th><th>器件数</th><th>墙体数</th><th>线缆数</th></tr>\n";
        for (const auto& floor : project->floors) {
            double floorCable = 0;
            for (const auto& c : floor.cables) floorCable += c.length_m;
            html << "<tr><td>" << floor.floorId << "</td><td>" << floor.floorName
                 << "</td><td>" << floor.devices.size() << "</td><td>" << floor.walls.size()
                 << "</td><td>" << floor.cables.size() << " (" << floorCable << "m)</td></tr>\n";
        }
        html << "</table>\n";

        // 器件清单
        if (m_config.includeDeviceList) {
            html << "<h2>三、器件清单</h2>\n";
            std::map<std::string, int> modelCount;
            std::map<std::string, std::string> modelName;
            for (const auto& floor : project->floors) {
                for (const auto& dev : floor.devices) {
                    modelCount[dev.modelId]++;
                }
            }
            for (const auto& m : project->deviceLibrary) {
                modelName[m.modelId] = m.displayName;
            }
            html << "<table>\n<tr><th>序号</th><th>型号</th><th>名称</th><th>数量</th></tr>\n";
            int idx = 1;
            for (const auto& kv : modelCount) {
                std::string name = modelName.count(kv.first) ? modelName[kv.first] : kv.first;
                html << "<tr><td>" << idx++ << "</td><td>" << kv.first
                     << "</td><td>" << name << "</td><td>" << kv.second << "</td></tr>\n";
            }
            html << "</table>\n";
        }

        // 链路预算结果
        if (m_config.includeLinkBudget && linkReport) {
            html << "<h2>四、链路预算结果</h2>\n";
            html << "<div class=\"summary-box\">\n";
            html << "<div class=\"metric\"><div class=\"metric-value\">" << linkReport->calculatedDevices
                 << "/" << linkReport->totalDevices << "</div><div class=\"metric-label\">已计算/总器件</div></div>\n";
            html << "<div class=\"metric\"><div class=\"metric-value success\">" << linkReport->avgAntennaPower_dBm
                 << " dBm</div><div class=\"metric-label\">平均天线功率</div></div>\n";
            html << "<div class=\"metric\"><div class=\"metric-value\">" << linkReport->minAntennaPower_dBm
                 << " ~ " << linkReport->maxAntennaPower_dBm << " dBm</div><div class=\"metric-label\">功率范围</div></div>\n";
            html << "<div class=\"metric\"><div class=\"metric-value error\">" << linkReport->errorCount
                 << "</div><div class=\"metric-label\">错误数</div></div>\n";
            html << "<div class=\"metric\"><div class=\"metric-value warning\">" << linkReport->warningCount
                 << "</div><div class=\"metric-label\">警告数</div></div>\n";
            html << "</div>\n";
        }

        // 造价概算
        if (m_config.includeCostEstimate && costSummary) {
            html << "<h2>五、工程造价概算</h2>\n";
            html << "<table>\n<tr><th>类别</th><th>项目</th><th>数量</th><th>单价(元)</th><th>总价(元)</th></tr>\n";
            for (const auto& item : costSummary->items) {
                html << "<tr><td>" << item.category << "</td><td>" << item.itemName
                     << "</td><td>" << item.quantity << "</td><td>" << item.unitPrice
                     << "</td><td>" << item.totalPrice << "</td></tr>\n";
            }
            html << "</table>\n";
            html << "<div class=\"summary-box\">\n";
            html << "<table>\n";
            html << "<tr><th>材料费</th><td>" << costSummary->materialCost << " 元</td></tr>\n";
            html << "<tr><th>线缆费</th><td>" << costSummary->cableCost << " 元</td></tr>\n";
            html << "<tr><th>人工费</th><td>" << costSummary->laborCost << " 元</td></tr>\n";
            html << "<tr><th>其他费</th><td>" << costSummary->otherCost << " 元</td></tr>\n";
            html << "<tr><th>小计</th><td>" << costSummary->subtotal << " 元</td></tr>\n";
            html << "<tr><th>税金(" << (costSummary->taxRate*100) << "%)</th><td>" << costSummary->tax << " 元</td></tr>\n";
            html << "<tr><th class=\"cost-total\">含税总价</th><td class=\"cost-total\">" << costSummary->total << " 元</td></tr>\n";
            html << "</table>\n</div>\n";
        }

        // 页脚
        html << "<div class=\"footer\">\n";
        html << "<p>本报告由智分Design V3.1.0 自动生成 | 生成时间: " << getCurrentTime() << "</p>\n";
        html << "<p>注：本报告数据仅供参考，实际造价以采购合同和施工图预算为准。</p>\n";
        html << "</div>\n";

        html << "</body>\n</html>\n";
        outHtml = html.str();
        return ZF_ERR_OK;
    }

private:
    ReportConfig m_config;

    std::string getCurrentTime() const {
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        char buf[64];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);
        return std::string(buf);
    }
};

} // namespace zf
