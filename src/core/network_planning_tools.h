#ifndef NETWORK_PLANNING_TOOLS_H
#define NETWORK_PLANNING_TOOLS_H

#include <QString>
#include <QList>
#include <QMap>
#include <QVector>

namespace Zhifen {

// 网络制式
enum NetworkStandard {
    Net_2G_GSM = 0,
    Net_3G_WCDMA = 1,
    Net_4G_LTE = 2,
    Net_5G_NR = 3
};

// 频段配置
struct FrequencyBand {
    NetworkStandard standard;
    QString bandName;       // 如"GSM900"、"LTE1800"、"NR3500"
    qreal uplinkLow;        // 上行低频MHz
    qreal uplinkHigh;       // 上行高频MHz
    qreal downlinkLow;      // 下行低频MHz
    qreal downlinkHigh;     // 下行高频MHz
    qreal bandwidth;        // 带宽MHz
    QString duplex;         // 双工方式FDD/TDD
    QString operatorName;   // 运营商
};

// 干扰等级
enum InterferenceLevel {
    Interf_None = 0,        // 无干扰
    Interf_Weak = 1,        // 弱干扰
    Interf_Medium = 2,      // 中干扰
    Interf_Strong = 3       // 强干扰
};

// 干扰结果
struct InterferenceResult {
    QString sourceSystem;    // 源系统
    QString victimSystem;    // 受害系统
    InterferenceLevel level; // 干扰等级
    qreal isolationRequired; // 所需隔离度dB
    qreal spuriousEmission;  // 杂散发射dBm
    qreal intermodulation;   // 互调产物dBm
    QString description;     // 干扰描述
    QString recommendation;  // 建议措施
};

// 容量参数
struct CapacityParams {
    qreal area;              // 覆盖面积m²
    qreal userDensity;       // 用户密度人/km²
    qreal voiceTrafficPerUser; // 每用户语音业务Erl
    qreal dataTrafficPerUser;   // 每用户数据业务Mbps
    qreal penetrationRate;   // 渗透率
    int cellCount;           // 小区数
};

// 容量结果
struct CapacityResult {
    int totalUsers;          // 总用户数
    int usersPerCell;        // 每小区用户数
    qreal voiceTraffic;      // 语音业务Erl
    qreal dataTraffic;       // 数据业务Mbps
    qreal voiceCapacity;     // 语音容量Erl
    qreal dataCapacity;      // 数据容量Mbps
    qreal voiceUtilization;  // 语音利用率%
    qreal dataUtilization;   // 数据利用率%
    bool capacitySufficient; // 容量是否足够
    QString recommendation;  // 扩容建议
};

// PCI规划结果
struct PCIResult {
    int cellId;              // 小区ID
    int pci;                 // PCI值
    int mod3;                // 模3值
    int mod30;               // 模30值
    QString conflictStatus;  // 冲突状态
};

// 网络规划工具集
class NetworkPlanningTools
{
public:
    static NetworkPlanningTools& instance();

    // 频段配置
    QList<FrequencyBand> defaultBands() const;
    QList<FrequencyBand> bandsByOperator(const QString &operatorName) const;
    FrequencyBand bandByName(const QString &name) const;

    // 干扰分析
    QList<InterferenceResult> analyzeInterference(const QList<FrequencyBand> &bands);
    InterferenceResult analyzePair(const FrequencyBand &source, const FrequencyBand &victim);
    QString interferenceMatrix(const QList<FrequencyBand> &bands) const;

    // 容量规划
    CapacityResult calculateCapacity(NetworkStandard standard, const CapacityParams &params);
    QList<CapacityResult> multiStandardCapacity(const QList<NetworkStandard> &standards,
                                                   const CapacityParams &params);

    // 频率规划
    QString frequencyPlanReport(const QList<FrequencyBand> &bands) const;

    // PCI规划
    QList<PCIResult> planPCI(int cellCount, const QList<int> &neighborRelations = QList<int>());
    bool checkPCIConflict(const QList<PCIResult> &pcis) const;

    // 综合报告
    QString fullReport(const QList<FrequencyBand> &bands, const CapacityParams &params, int cellCount);

private:
    NetworkPlanningTools();
    QList<FrequencyBand> m_defaultBands;

    qreal calculateSpurious(const FrequencyBand &source, const FrequencyBand &victim) const;
    qreal calculateIntermodulation(const FrequencyBand &band1, const FrequencyBand &band2) const;
    qreal calculateBlocking(const FrequencyBand &source, const FrequencyBand &victim) const;
    InterferenceLevel determineLevel(qreal isolation) const;
    qreal erlangB(int channels, qreal blockingRate) const;
};

} // namespace Zhifen

#endif // NETWORK_PLANNING_TOOLS_H
