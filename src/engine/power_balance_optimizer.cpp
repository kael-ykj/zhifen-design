#include "power_balance_optimizer.h"
#include "../devices/deviceitem.h"
#include "../entities/caditem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QtMath>
#include <QDebug>
#include <QRandomGenerator>

namespace Zhifen {

PowerBalanceOptimizer::PowerBalanceOptimizer() {}
PowerBalanceOptimizer::~PowerBalanceOptimizer() {}

qreal PowerBalanceOptimizer::calculateAverage(const QList<qreal> &values) {
    if (values.isEmpty()) return 0;
    qreal sum = 0;
    for (qreal v : values) sum += v;
    return sum / values.size();
}

qreal PowerBalanceOptimizer::calculateStdDev(const QList<qreal> &values) {
    if (values.size() < 2) return 0;
    qreal avg = calculateAverage(values);
    qreal sum = 0;
    for (qreal v : values) {
        sum += qPow(v - avg, 2);
    }
    return qSqrt(sum / (values.size() - 1));
}

QList<AntennaPowerInfo> PowerBalanceOptimizer::collectAntennas() {
    QList<AntennaPowerInfo> antennas;
    if (!m_scene) return antennas;

    int idx = 1;
    for (auto *item : m_scene->items()) {
        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            DeviceType dt = dev->deviceType();
            if (dt >= DevAntennaOmni && dt <= DevAntennaGrid) {
                AntennaPowerInfo info;
                info.deviceId = QString("ANT-%1").arg(idx++, 3, 10, QChar('0'));
                info.position = dev->pos();
                // 简化：模拟当前功率（实际应从链路预算获取）
                info.currentPower = m_config.targetPower + (QRandomGenerator::global()->generateDouble() - 0.5) * 10;
                info.targetPower = m_config.targetPower;
                info.deviation = info.currentPower - info.targetPower;
                info.pass = qAbs(info.deviation) <= m_config.maxDeviation &&
                            info.currentPower >= m_config.minPower &&
                            info.currentPower <= m_config.maxPower;
                antennas.append(info);
            }
        }
    }
    return antennas;
}

QList<AntennaPowerInfo> PowerBalanceOptimizer::analyzeCurrentPower() {
    return collectAntennas();
}

QList<AntennaPowerInfo> PowerBalanceOptimizer::simulateOptimization(const QList<AntennaPowerInfo> &antennas) {
    QList<AntennaPowerInfo> result = antennas;

    if (result.isEmpty()) return result;

    // 计算当前平均功率
    QList<qreal> powers;
    for (const auto &a : result) powers.append(a.currentPower);
    qreal avg = calculateAverage(powers);

    // 根据优化目标调整
    if (m_config.goal == Goal_Uniform) {
        // 均匀分配：所有天线调整到平均功率
        for (auto &a : result) {
            a.currentPower = avg + (QRandomGenerator::global()->generateDouble() - 0.5) * 1.0; // 小范围波动
            a.deviation = a.currentPower - a.targetPower;
            a.pass = qAbs(a.deviation) <= m_config.maxDeviation &&
                     a.currentPower >= m_config.minPower &&
                     a.currentPower <= m_config.maxPower;
        }
    } else if (m_config.goal == Goal_MaxPower) {
        // 最大功率：提升弱功率天线
        qreal max = *std::max_element(powers.begin(), powers.end());
        for (auto &a : result) {
            if (a.currentPower < max - 2) {
                a.currentPower = max - (QRandomGenerator::global()->generateDouble() * 1.5);
            }
            a.deviation = a.currentPower - a.targetPower;
            a.pass = qAbs(a.deviation) <= m_config.maxDeviation &&
                     a.currentPower >= m_config.minPower &&
                     a.currentPower <= m_config.maxPower;
        }
    } else if (m_config.goal == Goal_MinLoss) {
        // 最小损耗：保持当前分布，仅调整严重偏差
        for (auto &a : result) {
            if (qAbs(a.deviation) > m_config.maxDeviation * 2) {
                a.currentPower = a.targetPower + (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0;
            }
            a.deviation = a.currentPower - a.targetPower;
            a.pass = qAbs(a.deviation) <= m_config.maxDeviation &&
                     a.currentPower >= m_config.minPower &&
                     a.currentPower <= m_config.maxPower;
        }
    }

    return result;
}

QString PowerBalanceOptimizer::suggestSplitter(qreal requiredLoss) {
    if (requiredLoss <= 3.5) return "二功分器(3.5dB)";
    if (requiredLoss <= 5.5) return "三功分器(5.5dB)";
    if (requiredLoss <= 7.0) return "四功分器(7.0dB)";
    return "多级功分组合";
}

QString PowerBalanceOptimizer::suggestCoupler(qreal requiredCoupling) {
    if (requiredCoupling <= 5) return "5dB耦合器";
    if (requiredCoupling <= 6) return "6dB耦合器";
    if (requiredCoupling <= 7) return "7dB耦合器";
    if (requiredCoupling <= 10) return "10dB耦合器";
    if (requiredCoupling <= 12) return "12dB耦合器";
    if (requiredCoupling <= 15) return "15dB耦合器";
    if (requiredCoupling <= 20) return "20dB耦合器";
    if (requiredCoupling <= 30) return "30dB耦合器";
    return "40dB耦合器";
}

QList<AdjustmentSuggestion> PowerBalanceOptimizer::generateSuggestions(
    const QList<AntennaPowerInfo> &before, const QList<AntennaPowerInfo> &after) {
    QList<AdjustmentSuggestion> suggestions;

    for (int i = 0; i < before.size() && i < after.size(); i++) {
        qreal diff = after[i].currentPower - before[i].currentPower;
        if (qAbs(diff) > 0.5) {
            AdjustmentSuggestion sug;
            sug.deviceId = before[i].deviceId;
            if (diff > 0) {
                // 需要提升功率，建议减小损耗
                sug.currentType = "当前配置";
                sug.suggestedType = suggestCoupler(qAbs(diff) + 5);
                sug.currentLoss = 10.0;
                sug.suggestedLoss = 10.0 - diff;
                sug.reason = QString("功率偏低%1dB，建议更换耦合度更小的耦合器").arg(diff, 0, 'f', 1);
            } else {
                // 需要降低功率，建议增加损耗
                sug.currentType = "当前配置";
                sug.suggestedType = suggestSplitter(qAbs(diff));
                sug.currentLoss = 3.5;
                sug.suggestedLoss = 3.5 + qAbs(diff);
                sug.reason = QString("功率偏高%1dB，建议增加功分器或耦合度更大的耦合器").arg(qAbs(diff), 0, 'f', 1);
            }
            suggestions.append(sug);
        }
    }

    return suggestions;
}

OptimizationResult PowerBalanceOptimizer::optimize() {
    OptimizationResult result;

    // 1. 收集天线信息
    QList<AntennaPowerInfo> before = collectAntennas();
    if (before.isEmpty()) {
        result.success = false;
        result.warnings.append("未找到天线，无法优化");
        return result;
    }

    result.totalAntennas = before.size();
    result.beforePower = before;

    // 2. 计算优化前统计
    QList<qreal> beforePowers;
    for (const auto &a : before) {
        beforePowers.append(a.currentPower);
        if (a.pass) result.passCountBefore++;
    }
    result.beforeAvg = calculateAverage(beforePowers);
    result.beforeStdDev = calculateStdDev(beforePowers);

    // 3. 模拟优化
    QList<AntennaPowerInfo> after = simulateOptimization(before);
    result.afterPower = after;

    // 4. 计算优化后统计
    QList<qreal> afterPowers;
    for (const auto &a : after) {
        afterPowers.append(a.currentPower);
        if (a.pass) result.passCountAfter++;
    }
    result.afterAvg = calculateAverage(afterPowers);
    result.afterStdDev = calculateStdDev(afterPowers);

    // 5. 生成调整建议
    result.suggestions = generateSuggestions(before, after);

    // 6. 生成报告
    result.report = generateReport(result);
    result.success = true;

    return result;
}

QString PowerBalanceOptimizer::generateReport(const OptimizationResult &result) {
    QString report;
    report += "===== 功率平衡优化报告 =====\n\n";
    report += QString("优化频段: %1\n").arg(CoverageSimulator::bandName(m_config.band));
    report += QString("优化目标: %1\n\n").arg(
        m_config.goal == Goal_Uniform ? "均匀分配" :
        m_config.goal == Goal_MinLoss ? "最小损耗" :
        m_config.goal == Goal_MaxPower ? "最大功率" : "自定义");

    report += "--- 优化前 ---\n";
    report += QString("天线总数: %1\n").arg(result.totalAntennas);
    report += QString("达标数量: %1 (%2%)\n").arg(result.passCountBefore)
        .arg(result.totalAntennas > 0 ? result.passCountBefore * 100.0 / result.totalAntennas : 0, 0, 'f', 1);
    report += QString("平均功率: %1 dBm\n").arg(result.beforeAvg, 0, 'f', 2);
    report += QString("功率标准差: %1 dB\n\n").arg(result.beforeStdDev, 0, 'f', 2);

    report += "--- 优化后 ---\n";
    report += QString("达标数量: %1 (%2%)\n").arg(result.passCountAfter)
        .arg(result.totalAntennas > 0 ? result.passCountAfter * 100.0 / result.totalAntennas : 0, 0, 'f', 1);
    report += QString("平均功率: %1 dBm\n").arg(result.afterAvg, 0, 'f', 2);
    report += QString("功率标准差: %1 dB\n\n").arg(result.afterStdDev, 0, 'f', 2);

    report += "--- 优化效果 ---\n";
    report += QString("标准差降低: %1 dB\n").arg(result.beforeStdDev - result.afterStdDev, 0, 'f', 2);
    report += QString("达标率提升: %1%\n\n").arg(
        result.totalAntennas > 0 ? (result.passCountAfter - result.passCountBefore) * 100.0 / result.totalAntennas : 0, 0, 'f', 1);

    if (!result.suggestions.isEmpty()) {
        report += "--- 调整建议 ---\n";
        for (int i = 0; i < result.suggestions.size() && i < 10; i++) {
            const auto &s = result.suggestions[i];
            report += QString("%1. %2: %3\n").arg(i + 1).arg(s.deviceId).arg(s.reason);
            report += QString("   建议: %1\n").arg(s.suggestedType);
        }
        if (result.suggestions.size() > 10) {
            report += QString("...等共%1条建议\n").arg(result.suggestions.size());
        }
    }

    return report;
}

int PowerBalanceOptimizer::applySuggestions(const OptimizationResult &result) {
    if (!m_scene || !m_config.autoAdjust) return 0;
    // 简化：实际应根据建议修改场景中的器件
    return result.suggestions.size();
}

} // namespace Zhifen
