#include "special_design_tools.h"
#include "coverage_simulator.h"
#include <QtMath>
#include <QStringList>

namespace Zhifen {

// ==================== 电梯覆盖工具 ====================
ElevatorResult ElevatorCoverageTool::calculate(const ElevatorParams &params) {
    ElevatorResult result;

    if (params.floorCount <= 0 || params.floorHeight <= 0) {
        result.success = false;
        return result;
    }

    qreal totalHeight = params.floorCount * params.floorHeight;

    // 电梯井覆盖：使用对数周期天线，每3-4层一个天线
    // 天线覆盖范围约10-12米（垂直）
    qreal coveragePerAntenna = 12.0; // 米
    result.antennaCount = qCeil(totalHeight / coveragePerAntenna);
    result.antennaSpacing = totalHeight / result.antennaCount;
    result.antennaType = "对数周期天线(8-10dBi)";

    // 计算天线位置（从下往上均匀分布）
    for (int i = 0; i < result.antennaCount; i++) {
        qreal height = (i + 0.5) * result.antennaSpacing;
        result.antennaPositions.append(QPointF(params.shaftWidth / 2, height));
    }

    // 简化功率计算：基于自由空间损耗+多径
    qreal freq = CoverageSimulator::bandFrequency(params.band);
    qreal fspl = 20.0 * qLn(coveragePerAntenna / 2) / qLn(10.0) +
                 20.0 * qLn(freq) / qLn(10.0) - 27.55;

    result.maxPower = params.txPower + params.antennaGain - fspl - 5; // 最近点
    result.minPower = params.txPower + params.antennaGain - fspl - 15; // 最远点+多径
    result.avgPower = (result.maxPower + result.minPower) / 2;

    result.success = result.minPower >= params.targetPower;
    result.report = generateReport(result, params);

    return result;
}

QString ElevatorCoverageTool::generateReport(const ElevatorResult &result, const ElevatorParams &params) {
    QString report;
    report += "===== 电梯覆盖设计方案 =====\n\n";
    report += "--- 输入参数 ---\n";
    report += QString("楼层数: %1层\n").arg(params.floorCount);
    report += QString("层高: %1米\n").arg(params.floorHeight);
    report += QString("电梯井尺寸: %1 x %2米\n").arg(params.shaftWidth).arg(params.shaftDepth);
    report += QString("总高度: %1米\n").arg(params.floorCount * params.floorHeight);
    report += QString("频段: %1\n").arg(CoverageSimulator::bandName(params.band));
    report += QString("发射功率: %1 dBm\n").arg(params.txPower);
    report += QString("天线增益: %1 dBi\n\n").arg(params.antennaGain);

    report += "--- 设计结果 ---\n";
    report += QString("推荐天线: %1\n").arg(result.antennaType);
    report += QString("天线数量: %1个\n").arg(result.antennaCount);
    report += QString("天线间距: %1米\n").arg(result.antennaSpacing, 0, 'f', 1);
    report += QString("平均覆盖功率: %1 dBm\n").arg(result.avgPower, 0, 'f', 1);
    report += QString("最强覆盖: %1 dBm\n").arg(result.maxPower, 0, 'f', 1);
    report += QString("最弱覆盖: %1 dBm\n").arg(result.minPower, 0, 'f', 1);
    report += QString("目标功率: %1 dBm\n\n").arg(params.targetPower);

    report += "--- 天线位置 ---\n";
    for (int i = 0; i < result.antennaPositions.size(); i++) {
        report += QString("天线%1: 高度%2米\n").arg(i + 1)
            .arg(result.antennaPositions[i].y(), 0, 'f', 1);
    }

    report += "\n--- 结论 ---\n";
    if (result.success) {
        report += "覆盖达标，方案可行。\n";
    } else {
        report += "覆盖不达标，建议增加天线数量或提高发射功率。\n";
    }

    return report;
}

// ==================== 漏缆分段工具 ====================
LeakyCableResult LeakyCableTool::calculate(const LeakyCableParams &params) {
    LeakyCableResult result;

    if (params.totalLength <= 0) {
        result.success = false;
        return result;
    }

    // 计算总传输损耗
    result.totalLoss = params.transmissionLoss * params.totalLength / 100.0;
    result.endPower = params.txPower - result.totalLoss;

    // 分段策略：每段末端功率不低于minPower
    qreal maxSegmentLoss = params.txPower - params.minPower;
    qreal maxSegmentLength = maxSegmentLoss / (params.transmissionLoss / 100.0);
    maxSegmentLength = qMin(maxSegmentLength, 50.0); // 单段最长50米

    result.segmentCount = qCeil(params.totalLength / maxSegmentLength);
    qreal segmentLength = params.totalLength / result.segmentCount;

    qreal currentPower = params.txPower;
    for (int i = 0; i < result.segmentCount; i++) {
        LeakyCableSegment seg;
        seg.segmentIndex = i + 1;
        seg.startPos = i * segmentLength;
        seg.endPos = (i + 1) * segmentLength;
        seg.length = segmentLength;
        seg.inputPower = currentPower;

        qreal segLoss = params.transmissionLoss * segmentLength / 100.0;
        seg.outputPower = currentPower - segLoss;
        seg.avgCoupledPower = (seg.inputPower + seg.outputPower) / 2 - params.couplingLoss;
        seg.pass = seg.avgCoupledPower >= params.targetPower;

        result.segments.append(seg);
        currentPower = seg.outputPower;
    }

    result.success = result.endPower >= params.minPower - params.couplingLoss;
    result.report = generateReport(result, params);

    return result;
}

QString LeakyCableTool::generateReport(const LeakyCableResult &result, const LeakyCableParams &params) {
    QString report;
    report += "===== 漏缆分段设计方案 =====\n\n";
    report += "--- 输入参数 ---\n";
    report += QString("漏缆总长度: %1米\n").arg(params.totalLength);
    report += QString("耦合损耗: %1 dB\n").arg(params.couplingLoss);
    report += QString("传输损耗: %1 dB/100m\n").arg(params.transmissionLoss);
    report += QString("输入功率: %1 dBm\n").arg(params.txPower);
    report += QString("频段: %1\n\n").arg(CoverageSimulator::bandName(params.band));

    report += "--- 设计结果 ---\n";
    report += QString("分段数量: %1段\n").arg(result.segmentCount);
    report += QString("总损耗: %1 dB\n").arg(result.totalLoss, 0, 'f', 1);
    report += QString("末端功率: %1 dBm\n\n").arg(result.endPower, 0, 'f', 1);

    report += "--- 分段详情 ---\n";
    for (const auto &seg : result.segments) {
        report += QString("第%1段: %2-%3米(长%4米)\n")
            .arg(seg.segmentIndex).arg(seg.startPos, 0, 'f', 0)
            .arg(seg.endPos, 0, 'f', 0).arg(seg.length, 0, 'f', 1);
        report += QString("  输入: %1dBm  输出: %2dBm  平均耦合: %3dBm  %4\n")
            .arg(seg.inputPower, 0, 'f', 1).arg(seg.outputPower, 0, 'f', 1)
            .arg(seg.avgCoupledPower, 0, 'f', 1)
            .arg(seg.pass ? "达标" : "不达标");
    }

    report += "\n--- 结论 ---\n";
    if (result.success) {
        report += "漏缆功率分布达标，方案可行。\n";
    } else {
        report += "末端功率不足，建议增加分段或使用中继放大器。\n";
    }

    return report;
}

// ==================== 楼间对打工具 ====================
BuildingToBuildingResult BuildingToBuildingTool::calculate(const BuildingToBuildingParams &params) {
    BuildingToBuildingResult result;

    if (params.distance <= 0) {
        result.success = false;
        return result;
    }

    qreal freq = CoverageSimulator::bandFrequency(params.band);

    // 自由空间损耗
    result.freeSpaceLoss = 20.0 * qLn(params.distance) / qLn(10.0) +
                           20.0 * qLn(freq) / qLn(10.0) - 27.55;

    // 接收功率 = 发射功率 + 发射天线增益 + 接收天线增益 - 自由空间损耗 - 建筑物穿透损耗
    result.receivedPower = params.txPower + params.txAntennaGain + params.rxAntennaGain -
                           result.freeSpaceLoss - params.buildingPenetration;

    // 最大覆盖距离（达到目标功率的距离）
    qreal maxLoss = params.txPower + params.txAntennaGain + params.rxAntennaGain -
                    params.buildingPenetration - params.targetPower;
    // FSPL = 20log(d) + 20log(f) - 27.55
    // d = 10^((FSPL - 20log(f) + 27.55) / 20)
    result.maxCoverageDistance = qPow(10, (maxLoss - 20.0 * qLn(freq) / qLn(10.0) + 27.55) / 20.0);

    result.coveragePass = result.receivedPower >= params.targetPower;

    // 推荐天线
    if (params.distance > 100) {
        result.suggestedAntenna = "板状天线(15-18dBi)";
    } else if (params.distance > 50) {
        result.suggestedAntenna = "射灯天线(12-15dBi)";
    } else {
        result.suggestedAntenna = "外引天线(8-12dBi)";
    }

    result.success = true;
    result.report = generateReport(result, params);

    return result;
}

QString BuildingToBuildingTool::generateReport(const BuildingToBuildingResult &result, const BuildingToBuildingParams &params) {
    QString report;
    report += "===== 楼间对打设计方案 =====\n\n";
    report += "--- 输入参数 ---\n";
    report += QString("楼间距: %1米\n").arg(params.distance);
    report += QString("发射天线高度: %1米\n").arg(params.txHeight);
    report += QString("接收点高度: %1米\n").arg(params.rxHeight);
    report += QString("发射功率: %1 dBm\n").arg(params.txPower);
    report += QString("发射天线增益: %1 dBi\n").arg(params.txAntennaGain);
    report += QString("接收天线增益: %1 dBi\n").arg(params.rxAntennaGain);
    report += QString("建筑物穿透损耗: %1 dB\n").arg(params.buildingPenetration);
    report += QString("频段: %1\n\n").arg(CoverageSimulator::bandName(params.band));

    report += "--- 计算结果 ---\n";
    report += QString("自由空间损耗: %1 dB\n").arg(result.freeSpaceLoss, 0, 'f', 1);
    report += QString("接收功率: %1 dBm\n").arg(result.receivedPower, 0, 'f', 1);
    report += QString("目标功率: %1 dBm\n").arg(params.targetPower);
    report += QString("最大覆盖距离: %1米\n\n").arg(result.maxCoverageDistance, 0, 'f', 0);

    report += "--- 天线选型 ---\n";
    report += QString("当前天线: %1\n").arg(params.antennaType);
    report += QString("推荐天线: %1\n\n").arg(result.suggestedAntenna);

    report += "--- 结论 ---\n";
    if (result.coveragePass) {
        report += QString("覆盖达标，当前距离%1米在最大覆盖距离%2米内。\n")
            .arg(params.distance).arg(result.maxCoverageDistance, 0, 'f', 0);
    } else {
        report += QString("覆盖不达标，当前距离%1米超过最大覆盖距离%2米，建议更换高增益天线或增加发射功率。\n")
            .arg(params.distance).arg(result.maxCoverageDistance, 0, 'f', 0);
    }

    return report;
}

} // namespace Zhifen
