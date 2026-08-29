#include "network_planning_tools.h"
#include <QtMath>
#include <QTextStream>

namespace Zhifen {

NetworkPlanningTools::NetworkPlanningTools() {
    // 初始化默认频段配置（中国移动/电信/联通）
    m_defaultBands = {
        {Net_2G_GSM, "GSM900", 890, 915, 935, 960, 25, "FDD", "中国移动"},
        {Net_2G_GSM, "GSM1800", 1710, 1735, 1805, 1830, 25, "FDD", "中国移动"},
        {Net_3G_WCDMA, "WCDMA2100", 1920, 1980, 2110, 2170, 60, "FDD", "中国联通"},
        {Net_4G_LTE, "LTE1800", 1710, 1785, 1805, 1880, 75, "FDD", "中国电信"},
        {Net_4G_LTE, "LTE2300", 2300, 2390, 2300, 2390, 90, "TDD", "中国移动"},
        {Net_4G_LTE, "LTE2600", 2500, 2570, 2620, 2690, 70, "TDD", "中国移动"},
        {Net_5G_NR, "NR2.6G", 2515, 2675, 2515, 2675, 160, "TDD", "中国移动"},
        {Net_5G_NR, "NR3.5G", 3400, 3500, 3400, 3500, 100, "TDD", "中国电信"},
        {Net_5G_NR, "NR3.5G_联通", 3500, 3600, 3500, 3600, 100, "TDD", "中国联通"},
        {Net_5G_NR, "NR4.9G", 4800, 4900, 4800, 4900, 100, "TDD", "中国移动"},
    };
}

NetworkPlanningTools& NetworkPlanningTools::instance() {
    static NetworkPlanningTools inst;
    return inst;
}

QList<FrequencyBand> NetworkPlanningTools::defaultBands() const {
    return m_defaultBands;
}

QList<FrequencyBand> NetworkPlanningTools::bandsByOperator(const QString &operatorName) const {
    QList<FrequencyBand> result;
    for (const NetworkFrequencyBand &band : m_defaultBands) {
        if (band.operatorName == operatorName) result.append(band);
    }
    return result;
}

NetworkFrequencyBand NetworkPlanningTools::bandByName(const QString &name) const {
    for (const NetworkFrequencyBand &band : m_defaultBands) {
        if (band.bandName == name) return band;
    }
    return FrequencyBand();
}

qreal NetworkPlanningTools::calculateSpurious(const NetworkFrequencyBand &source, const NetworkFrequencyBand &victim) const {
    // 简化的杂散干扰计算：基于频段间隔和发射功率
    qreal freqGap = qAbs(source.downlinkHigh - victim.uplinkLow);
    if (freqGap > 500) return -100; // 频段间隔大，杂散可忽略
    qreal spurious = -30 - freqGap * 0.1; // 简化模型
    return qMax(spurious, -96.0);
}

qreal NetworkPlanningTools::calculateIntermodulation(const NetworkFrequencyBand &band1, const NetworkFrequencyBand &band2) const {
    // 简化的互调干扰计算：三阶互调 2f1-f2
    qreal f1 = (band1.uplinkLow + band1.uplinkHigh) / 2;
    qreal f2 = (band2.uplinkLow + band2.uplinkHigh) / 2;
    qreal im3 = 2 * f1 - f2;
    // 检查互调产物是否落入接收频段
    if (im3 >= band2.downlinkLow && im3 <= band2.downlinkHigh) {
        return -70; // 存在互调干扰
    }
    return -120;
}

qreal NetworkPlanningTools::calculateBlocking(const NetworkFrequencyBand &source, const NetworkFrequencyBand &victim) const {
    // 简化的阻塞干扰计算
    qreal freqGap = qAbs(source.downlinkHigh - victim.uplinkLow);
    if (freqGap < 100) return -50; // 近频阻塞
    if (freqGap < 300) return -70;
    return -100;
}

InterferenceLevel NetworkPlanningTools::determineLevel(qreal isolation) const {
    if (isolation >= 80) return Interf_None;
    if (isolation >= 60) return Interf_Weak;
    if (isolation >= 40) return Interf_Medium;
    return Interf_Strong;
}

InterferenceResult NetworkPlanningTools::analyzePair(const NetworkFrequencyBand &source, const NetworkFrequencyBand &victim) {
    InterferenceResult result;
    result.sourceSystem = source.bandName;
    result.victimSystem = victim.bandName;

    qreal spurious = calculateSpurious(source, victim);
    qreal intermod = calculateIntermodulation(source, victim);
    qreal blocking = calculateBlocking(source, victim);

    result.spuriousEmission = spurious;
    result.intermodulation = intermod;

    qreal worst = qMax(qMax(spurious, intermod), blocking);
    result.isolationRequired = qMax(0.0, -worst - 30); // 所需隔离度
    result.level = determineLevel(result.isolationRequired);

    QString levelStr;
    switch (result.level) {
        case Interf_None: levelStr = "无干扰"; break;
        case Interf_Weak: levelStr = "弱干扰"; break;
        case Interf_Medium: levelStr = "中干扰"; break;
        case Interf_Strong: levelStr = "强干扰"; break;
    }
    result.description = QString("%1对%2产生%3，杂散%4dBm，互调%5dBm")
        .arg(source.bandName).arg(victim.bandName).arg(levelStr)
        .arg(spurious).arg(intermod);

    if (result.level == Interf_None) {
        result.recommendation = "无需额外隔离措施";
    } else if (result.level == Interf_Weak) {
        result.recommendation = "建议增加30dB隔离度，可通过空间隔离或滤波器实现";
    } else if (result.level == Interf_Medium) {
        result.recommendation = "建议增加60dB隔离度，需使用高性能滤波器或增加物理距离";
    } else {
        result.recommendation = "严重干扰！建议重新规划频段或使用高性能合路器+滤波器";
    }

    return result;
}

QList<InterferenceResult> NetworkPlanningTools::analyzeInterference(const QList<FrequencyBand> &bands) {
    QList<InterferenceResult> results;
    for (int i = 0; i < bands.size(); i++) {
        for (int j = 0; j < bands.size(); j++) {
            if (i != j) {
                results.append(analyzePair(bands[i], bands[j]));
            }
        }
    }
    return results;
}

QString NetworkPlanningTools::interferenceMatrix(const QList<FrequencyBand> &bands) const {
    QString result;
    QTextStream stream(&result);
    stream << "=== 系统间干扰矩阵 ===\n\n";
    stream << "源系统\\\\受害系统";
    for (const NetworkFrequencyBand &band : bands) {
        stream << "\t" << band.bandName;
    }
    stream << "\n";

    for (int i = 0; i < bands.size(); i++) {
        stream << bands[i].bandName;
        for (int j = 0; j < bands.size(); j++) {
            if (i == j) {
                stream << "\t-";
            } else {
                InterferenceResult r = const_cast<NetworkPlanningTools*>(this)->analyzePair(bands[i], bands[j]);
                QString symbol;
                switch (r.level) {
                    case Interf_None: symbol = "○"; break;
                    case Interf_Weak: symbol = "△"; break;
                    case Interf_Medium: symbol = "□"; break;
                    case Interf_Strong: symbol = "■"; break;
                }
                stream << "\t" << symbol;
            }
        }
        stream << "\n";
    }
    stream << "\n图例: ○无干扰 △弱干扰 □中干扰 ■强干扰\n";
    return result;
}

qreal NetworkPlanningTools::erlangB(int channels, qreal blockingRate) const {
    // 简化的Erlang B公式（数值近似）
    if (channels <= 0) return 0;
    qreal a = channels * 0.8; // 近似
    for (int i = 0; i < 10; i++) {
        qreal b = 1.0;
        for (int k = 1; k <= channels; k++) {
            b = 1.0 + k / a * b;
        }
        b = 1.0 / b;
        if (qAbs(b - blockingRate) < 0.001) break;
        a = a * (1 + (blockingRate - b) * 0.5);
    }
    return a;
}

CapacityResult NetworkPlanningTools::calculateCapacity(NetworkStandard standard, const CapacityParams &params) {
    CapacityResult result;
    result.totalUsers = qFloor(params.area / 1000000.0 * params.userDensity * params.penetrationRate);
    result.usersPerCell = qFloor(result.totalUsers / qMax(1, params.cellCount));
    result.voiceTraffic = result.usersPerCell * params.voiceTrafficPerUser;
    result.dataTraffic = result.usersPerCell * params.dataTrafficPerUser;

    // 各制式容量参数（简化模型）
    switch (standard) {
        case Net_2G_GSM:
            result.voiceCapacity = erlangB(8, 0.02); // 8个TRX
            result.dataCapacity = 0.5; // GPRS/EDGE
            break;
        case Net_3G_WCDMA:
            result.voiceCapacity = erlangB(32, 0.02);
            result.dataCapacity = 5.0; // HSPA
            break;
        case Net_4G_LTE:
            result.voiceCapacity = erlangB(64, 0.02); // VoLTE
            result.dataCapacity = 50.0; // 100MHz带宽
            break;
        case Net_5G_NR:
            result.voiceCapacity = erlangB(128, 0.02); // VoNR
            result.dataCapacity = 200.0; // 100MHz带宽，Massive MIMO
            break;
    }

    result.voiceUtilization = result.voiceCapacity > 0 ? result.voiceTraffic / result.voiceCapacity * 100 : 0;
    result.dataUtilization = result.dataCapacity > 0 ? result.dataTraffic / result.dataCapacity * 100 : 0;
    result.capacitySufficient = (result.voiceUtilization <= 70 && result.dataUtilization <= 70);

    if (!result.capacitySufficient) {
        if (result.voiceUtilization > 70) {
            result.recommendation = QString("语音容量不足！建议增加小区数（当前%1个，建议%2个）")
                .arg(params.cellCount).arg(qCeil(params.cellCount * result.voiceUtilization / 70.0));
        } else {
            result.recommendation = QString("数据容量不足！建议增加小区数（当前%1个，建议%2个）或增加载波")
                .arg(params.cellCount).arg(qCeil(params.cellCount * result.dataUtilization / 70.0));
        }
    } else {
        result.recommendation = "容量充足，无需扩容";
    }

    return result;
}

QList<CapacityResult> NetworkPlanningTools::multiStandardCapacity(const QList<NetworkStandard> &standards,
                                                                      const CapacityParams &params) {
    QList<CapacityResult> results;
    for (NetworkStandard std : standards) {
        results.append(calculateCapacity(std, params));
    }
    return results;
}

QString NetworkPlanningTools::frequencyPlanReport(const QList<FrequencyBand> &bands) const {
    QString result;
    QTextStream stream(&result);
    stream << "=== 频率规划报告 ===\n\n";
    for (const NetworkFrequencyBand &band : bands) {
        QString stdStr;
        switch (band.standard) {
            case Net_2G_GSM: stdStr = "2G GSM"; break;
            case Net_3G_WCDMA: stdStr = "3G WCDMA"; break;
            case Net_4G_LTE: stdStr = "4G LTE"; break;
            case Net_5G_NR: stdStr = "5G NR"; break;
        }
        stream << QString("[%1] %2 (%3)\n").arg(stdStr).arg(band.bandName).arg(band.operatorName);
        stream << QString("  上行: %1-%2 MHz\n").arg(band.uplinkLow).arg(band.uplinkHigh);
        stream << QString("  下行: %1-%2 MHz\n").arg(band.downlinkLow).arg(band.downlinkHigh);
        stream << QString("  带宽: %1 MHz  双工: %2\n\n").arg(band.bandwidth).arg(band.duplex);
    }
    return result;
}

QList<PCIResult> NetworkPlanningTools::planPCI(int cellCount, const QList<int> &neighborRelations) {
    QList<PCIResult> results;
    Q_UNUSED(neighborRelations);

    // 5G NR PCI范围0-1007，避免模3和模30冲突
    QList<int> usedMod3;
    QList<int> usedMod30;

    for (int i = 0; i < cellCount; i++) {
        PCIResult result;
        result.cellId = i;

        // 寻找无冲突的PCI
        int pci = i * 3; // 基础分配
        while (pci < 1008) {
            int mod3 = pci % 3;
            int mod30 = pci % 30;
            bool conflict = false;
            // 检查与已有小区的模3冲突（邻区不能同模3）
            for (int j = qMax(0, i - 3); j < i; j++) {
                if (results[j].mod3 == mod3) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) {
                result.pci = pci;
                result.mod3 = mod3;
                result.mod30 = mod30;
                result.conflictStatus = "正常";
                break;
            }
            pci++;
        }

        if (result.pci >= 1008) {
            result.pci = i % 1008;
            result.mod3 = result.pci % 3;
            result.mod30 = result.pci % 30;
            result.conflictStatus = "存在冲突，需手动调整";
        }

        results.append(result);
        usedMod3.append(result.mod3);
        usedMod30.append(result.mod30);
    }

    return results;
}

bool NetworkPlanningTools::checkPCIConflict(const QList<PCIResult> &pcis) const {
    for (int i = 0; i < pcis.size(); i++) {
        for (int j = i + 1; j < pcis.size(); j++) {
            // 邻区模3冲突检查
            if (qAbs(i - j) <= 3 && pcis[i].mod3 == pcis[j].mod3) {
                return true;
            }
        }
    }
    return false;
}

QString NetworkPlanningTools::fullReport(const QList<FrequencyBand> &bands, const CapacityParams &params, int cellCount) {
    QString result;
    QTextStream stream(&result);

    stream << "========================================\n";
    stream << "  智分Design 网络规划综合分析报告\n";
    stream << "========================================\n\n";

    // 频率规划
    stream << frequencyPlanReport(bands);

    // 干扰分析
    QList<InterferenceResult> interfResults = analyzeInterference(bands);
    stream << interferenceMatrix(bands);
    stream << "\n=== 干扰详情 ===\n";
    for (const InterferenceResult &r : interfResults) {
        if (r.level != Interf_None) {
            stream << r.description << "\n";
            stream << "  建议: " << r.recommendation << "\n";
        }
    }

    // 容量规划
    stream << "\n=== 容量规划 ===\n";
    QList<NetworkStandard> standards = {Net_2G_GSM, Net_4G_LTE, Net_5G_NR};
    QList<CapacityResult> capResults = multiStandardCapacity(standards, params);
    QStringList stdNames = {"2G GSM", "4G LTE", "5G NR"};
    for (int i = 0; i < capResults.size(); i++) {
        const CapacityResult &c = capResults[i];
        stream << QString("[%1]\n").arg(stdNames[i]);
        stream << QString("  总用户: %1  每小区: %2\n").arg(c.totalUsers).arg(c.usersPerCell);
        stream << QString("  语音: %1 Erl (容量%2, 利用率%3%)\n")
            .arg(c.voiceTraffic).arg(c.voiceCapacity).arg(c.voiceUtilization, 0, 'f', 1);
        stream << QString("  数据: %1 Mbps (容量%2, 利用率%3%)\n")
            .arg(c.dataTraffic).arg(c.dataCapacity).arg(c.dataUtilization, 0, 'f', 1);
        stream << "  结论: " << c.recommendation << "\n\n";
    }

    // PCI规划
    stream << "=== PCI规划 (5G NR) ===\n";
    QList<PCIResult> pciResults = planPCI(cellCount);
    for (const PCIResult &p : pciResults) {
        stream << QString("  小区%1: PCI=%2 (模3=%3, 模30=%4) %5\n")
            .arg(p.cellId).arg(p.pci).arg(p.mod3).arg(p.mod30).arg(p.conflictStatus);
    }
    stream << QString("  PCI冲突检查: %1\n").arg(checkPCIConflict(pciResults) ? "存在冲突" : "无冲突");

    stream << "\n========================================\n";
    stream << "  报告生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    stream << "========================================\n";

    return result;
}

} // namespace Zhifen
