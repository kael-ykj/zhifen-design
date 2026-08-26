#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {

// 单项造价明细
struct CostItem {
    std::string category;       // 类别（器件/线缆/人工/其他）
    std::string itemName;       // 项目名称
    std::string modelId;        // 型号ID
    int quantity{0};            // 数量
    double unitPrice{0.0};      // 单价（元）
    double totalPrice{0.0};     // 总价（元）
    std::string remark;         // 备注
};

// 造价汇总
struct CostSummary {
    double materialCost{0.0};   // 材料费
    double cableCost{0.0};      // 线缆费
    double laborCost{0.0};      // 人工费
    double otherCost{0.0};      // 其他费用
    double subtotal{0.0};       // 小计
    double taxRate{0.09};       // 税率9%
    double tax{0.0};            // 税金
    double total{0.0};          // 含税总价
    int deviceCount{0};         // 器件总数
    double cableLength_m{0.0};  // 线缆总长度
    std::vector<CostItem> items; // 明细列表
};

// 造价概算引擎
class CostEstimator {
public:
    CostEstimator() {
        initPriceTable();
    }

    void setLaborRate(double rate_per_point) { m_laborRate = rate_per_point; }
    void setTaxRate(double rate) { m_taxRate = rate; }

    // 从工程概算造价
    int estimateProject(const Project* project, CostSummary& outSummary) {
        if (!project) return ZF_ERR_ARG;

        outSummary = CostSummary();
        outSummary.taxRate = m_taxRate;
        std::map<std::string, int> modelCount;
        double totalCableLength = 0.0;

        // 统计器件数量和线缆长度
        for (const auto& floor : project->floors) {
            for (const auto& dev : floor.devices) {
                modelCount[dev.modelId]++;
                outSummary.deviceCount++;
            }
            for (const auto& cable : floor.cables) {
                totalCableLength += cable.length_m;
            }
        }

        // 计算器件费用
        for (const auto& kv : modelCount) {
            const std::string& modelId = kv.first;
            int qty = kv.second;
            double price = getDevicePrice(modelId);
            std::string name = getDeviceName(modelId, project);

            CostItem item;
            item.category = "器件";
            item.itemName = name;
            item.modelId = modelId;
            item.quantity = qty;
            item.unitPrice = price;
            item.totalPrice = price * qty;
            outSummary.items.push_back(item);
            outSummary.materialCost += item.totalPrice;
        }

        // 计算线缆费用
        outSummary.cableLength_m = totalCableLength;
        if (totalCableLength > 0) {
            CostItem cableItem;
            cableItem.category = "线缆";
            cableItem.itemName = "馈线/跳线";
            cableItem.quantity = (int)std::ceil(totalCableLength);
            cableItem.unitPrice = m_cablePricePerMeter;
            cableItem.totalPrice = totalCableLength * m_cablePricePerMeter;
            cableItem.remark = "按米计价";
            outSummary.items.push_back(cableItem);
            outSummary.cableCost = cableItem.totalPrice;
        }

        // 人工费（按器件点位计算）
        if (outSummary.deviceCount > 0) {
            CostItem laborItem;
            laborItem.category = "人工";
            laborItem.itemName = "安装调试费";
            laborItem.quantity = outSummary.deviceCount;
            laborItem.unitPrice = m_laborRate;
            laborItem.totalPrice = outSummary.deviceCount * m_laborRate;
            laborItem.remark = "按点位计算";
            outSummary.items.push_back(laborItem);
            outSummary.laborCost = laborItem.totalPrice;
        }

        // 其他费用（辅材、运输等，按材料费的10%）
        double other = outSummary.materialCost * 0.10;
        if (other > 0) {
            CostItem otherItem;
            otherItem.category = "其他";
            otherItem.itemName = "辅材及运输";
            otherItem.quantity = 1;
            otherItem.unitPrice = other;
            otherItem.totalPrice = other;
            otherItem.remark = "材料费的10%";
            outSummary.items.push_back(otherItem);
            outSummary.otherCost = other;
        }

        // 汇总
        outSummary.subtotal = outSummary.materialCost + outSummary.cableCost
                             + outSummary.laborCost + outSummary.otherCost;
        outSummary.tax = outSummary.subtotal * outSummary.taxRate;
        outSummary.total = outSummary.subtotal + outSummary.tax;

        return ZF_ERR_OK;
    }

    // 生成文本格式造价表
    std::string generateTextReport(const CostSummary& summary) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "========================================\n";
        oss << "       智分Design 工程造价概算表\n";
        oss << "========================================\n\n";
        oss << "工程概况:\n";
        oss << "  器件总数: " << summary.deviceCount << " 个\n";
        oss << "  线缆总长: " << summary.cableLength_m << " 米\n\n";
        oss << "----------------------------------------\n";
        oss << "序号 | 类别 | 项目 | 数量 | 单价 | 总价\n";
        oss << "----------------------------------------\n";
        int idx = 1;
        for (const auto& item : summary.items) {
            oss << idx++ << " | " << item.category << " | "
                << item.itemName << " | " << item.quantity
                << " | " << item.unitPrice << " | " << item.totalPrice << "\n";
        }
        oss << "----------------------------------------\n\n";
        oss << "费用汇总:\n";
        oss << "  材料费:   " << summary.materialCost << " 元\n";
        oss << "  线缆费:   " << summary.cableCost << " 元\n";
        oss << "  人工费:   " << summary.laborCost << " 元\n";
        oss << "  其他费:   " << summary.otherCost << " 元\n";
        oss << "  小计:     " << summary.subtotal << " 元\n";
        oss << "  税金(" << (summary.taxRate*100) << "%): " << summary.tax << " 元\n";
        oss << "========================================\n";
        oss << "  含税总价: " << summary.total << " 元\n";
        oss << "========================================\n";
        return oss.str();
    }

private:
    std::map<std::string, double> m_priceTable;
    double m_laborRate{150.0};      // 每点位人工费
    double m_cablePricePerMeter{8.0}; // 线缆每米价格
    double m_taxRate{0.09};          // 税率

    void initPriceTable() {
        // 信源类
        m_priceTable["SRC_FEMTO_200mW"] = 3500.0;
        m_priceTable["SRC_FEMTO_500mW"] = 5800.0;
        m_priceTable["SRC_FEMTO_1W"] = 8500.0;
        m_priceTable["SRC_REPEATER_2W"] = 12000.0;
        m_priceTable["SRC_BBU"] = 25000.0;
        m_priceTable["SRC_RRU"] = 15000.0;
        // 功分器
        m_priceTable["SPL_2WAY"] = 45.0;
        m_priceTable["SPL_3WAY"] = 65.0;
        m_priceTable["SPL_4WAY"] = 85.0;
        m_priceTable["SPL_6WAY"] = 120.0;
        // 耦合器
        m_priceTable["CPL_5dB"] = 55.0;
        m_priceTable["CPL_6dB"] = 55.0;
        m_priceTable["CPL_7dB"] = 60.0;
        m_priceTable["CPL_10dB"] = 65.0;
        m_priceTable["CPL_15dB"] = 75.0;
        m_priceTable["CPL_20dB"] = 85.0;
        // 合路器
        m_priceTable["CMB_2IN1"] = 180.0;
        m_priceTable["CMB_3IN1"] = 280.0;
        m_priceTable["CMB_4IN1"] = 380.0;
        // 天线
        m_priceTable["ANT_OMNI_3dBi"] = 35.0;
        m_priceTable["ANT_OMNI_5dBi"] = 55.0;
        m_priceTable["ANT_PANEL_7dBi"] = 80.0;
        m_priceTable["ANT_PANEL_9dBi"] = 120.0;
        m_priceTable["ANT_DIRECTIONAL_11dBi"] = 180.0;
        // 负载
        m_priceTable["LOAD_5W"] = 25.0;
        m_priceTable["LOAD_10W"] = 40.0;
        m_priceTable["LOAD_50W"] = 80.0;
        // 线缆
        m_priceTable["CAB_12D"] = 12.0;
        m_priceTable["CAB_12D-FEEDER"] = 15.0;
        m_priceTable["CAB_8D"] = 8.0;
        m_priceTable["CAB_JUMPER_1M"] = 15.0;
        m_priceTable["CAB_JUMPER_2M"] = 25.0;
        m_priceTable["CAB_JUMPER_3M"] = 35.0;
    }

    double getDevicePrice(const std::string& modelId) const {
        auto it = m_priceTable.find(modelId);
        if (it != m_priceTable.end()) return it->second;
        // 默认价格按类别估算
        return 100.0;
    }

    std::string getDeviceName(const std::string& modelId, const Project* project) const {
        if (project) {
            for (const auto& m : project->deviceLibrary) {
                if (m.modelId == modelId) return m.displayName;
            }
        }
        return modelId;
    }
};

} // namespace zf
