#ifndef SPECIAL_DESIGN_TOOLS_H
#define SPECIAL_DESIGN_TOOLS_H

#include <QString>
#include <QList>
#include <QPointF>

namespace Zhifen {

// ==================== 电梯覆盖工具 ====================
struct ElevatorParams {
    int floorCount = 20;          // 楼层数
    qreal floorHeight = 3.0;      // 层高(米)
    qreal shaftWidth = 2.5;       // 电梯井宽度(米)
    qreal shaftDepth = 2.5;       // 电梯井深度(米)
    qreal txPower = 15.0;         // 发射功率(dBm)
    qreal antennaGain = 8.0;      // 天线增益(dBi)
    qreal targetPower = -85.0;    // 目标接收功率(dBm)
    FrequencyBand band = Band_4G;  // 频段
};

struct ElevatorResult {
    bool success = false;
    int antennaCount = 0;         // 天线数量
    qreal antennaSpacing = 0;     // 天线间距(米)
    QString antennaType;          // 推荐天线类型
    QList<QPointF> antennaPositions; // 天线位置
    qreal avgPower = 0;           // 平均覆盖功率(dBm)
    qreal minPower = 0;           // 最弱覆盖功率(dBm)
    qreal maxPower = 0;           // 最强覆盖功率(dBm)
    QString report;               // 设计报告
};

class ElevatorCoverageTool {
public:
    static ElevatorResult calculate(const ElevatorParams &params);
    static QString generateReport(const ElevatorResult &result, const ElevatorParams &params);
};

// ==================== 漏缆分段工具 ====================
struct LeakyCableParams {
    qreal totalLength = 100.0;    // 漏缆总长度(米)
    qreal couplingLoss = 70.0;    // 耦合损耗(dB)
    qreal transmissionLoss = 2.5; // 传输损耗(dB/100m)
    qreal txPower = 43.0;         // 输入端功率(dBm)
    qreal targetPower = -80.0;    // 目标耦合输出功率(dBm)
    qreal minPower = -90.0;       // 最小允许功率(dBm)
    FrequencyBand band = Band_4G; // 频段
};

struct LeakyCableSegment {
    int segmentIndex = 0;
    qreal startPos = 0;           // 分段起始位置(米)
    qreal endPos = 0;             // 分段结束位置(米)
    qreal length = 0;             // 分段长度(米)
    qreal inputPower = 0;         // 输入端功率(dBm)
    qreal outputPower = 0;        // 输出端功率(dBm)
    qreal avgCoupledPower = 0;    // 平均耦合功率(dBm)
    bool pass = false;            // 是否达标
};

struct LeakyCableResult {
    bool success = false;
    int segmentCount = 0;         // 分段数量
    QList<LeakyCableSegment> segments;
    qreal endPower = 0;           // 末端功率(dBm)
    qreal totalLoss = 0;          // 总损耗(dB)
    QString report;               // 设计报告
};

class LeakyCableTool {
public:
    static LeakyCableResult calculate(const LeakyCableParams &params);
    static QString generateReport(const LeakyCableResult &result, const LeakyCableParams &params);
};

// ==================== 楼间对打工具 ====================
struct BuildingToBuildingParams {
    qreal distance = 50.0;        // 楼间距(米)
    qreal txHeight = 30.0;        // 发射天线高度(米)
    qreal rxHeight = 15.0;        // 接收点高度(米)
    qreal txPower = 43.0;         // 发射功率(dBm)
    qreal txAntennaGain = 12.0;   // 发射天线增益(dBi)
    qreal rxAntennaGain = 2.0;    // 接收天线增益(dBi)
    qreal buildingPenetration = 15.0; // 建筑物穿透损耗(dB)
    qreal targetPower = -90.0;    // 目标接收功率(dBm)
    FrequencyBand band = Band_4G; // 频段
    QString antennaType = "射灯天线"; // 天线类型
};

struct BuildingToBuildingResult {
    bool success = false;
    qreal freeSpaceLoss = 0;      // 自由空间损耗(dB)
    qreal receivedPower = 0;      // 接收功率(dBm)
    qreal maxCoverageDistance = 0; // 最大覆盖距离(米)
    bool coveragePass = false;    // 覆盖是否达标
    QString suggestedAntenna;     // 推荐天线类型
    QString report;               // 设计报告
};

class BuildingToBuildingTool {
public:
    static BuildingToBuildingResult calculate(const BuildingToBuildingParams &params);
    static QString generateReport(const BuildingToBuildingResult &result, const BuildingToBuildingParams &params);
};

} // namespace Zhifen

#endif // SPECIAL_DESIGN_TOOLS_H
