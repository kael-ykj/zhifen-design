#ifndef POWER_BALANCE_OPTIMIZER_H
#define POWER_BALANCE_OPTIMIZER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QGraphicsScene>
#include "coverage_simulator.h"

namespace Zhifen {

// 优化目标
enum OptimizationGoal {
    Goal_Uniform = 0,      // 均匀分配（功率最均衡）
    Goal_MinLoss = 1,      // 最小损耗
    Goal_MaxPower = 2,     // 最大功率输出
    Goal_Custom = 3        // 自定义目标
};

// 优化参数
struct OptimizationConfig {
    OptimizationGoal goal = Goal_Uniform;
    qreal targetPower = -10.0;      // 目标天线口功率(dBm)
    qreal maxDeviation = 3.0;       // 允许最大偏差(dB)
    qreal minPower = -15.0;         // 最小允许功率(dBm)
    qreal maxPower = -5.0;          // 最大允许功率(dBm)
    FrequencyBand band = Band_4G;   // 优化频段
    int maxIterations = 100;        // 最大迭代次数
    bool autoAdjust = true;         // 是否自动调整器件
};

// 天线功率信息
struct AntennaPowerInfo {
    QString deviceId;
    QPointF position;
    qreal currentPower = 0;     // 当前功率
    qreal targetPower = 0;      // 目标功率
    qreal deviation = 0;        // 偏差
    bool pass = false;          // 是否达标
};

// 调整建议
struct AdjustmentSuggestion {
    QString deviceId;           // 器件编号
    QString currentType;        // 当前型号
    QString suggestedType;      // 建议型号
    qreal currentLoss = 0;      // 当前损耗
    qreal suggestedLoss = 0;    // 建议损耗
    QString reason;             // 调整原因
};

// 优化结果
struct OptimizationResult {
    bool success = false;
    QList<AntennaPowerInfo> beforePower;   // 优化前功率
    QList<AntennaPowerInfo> afterPower;    // 优化后功率
    QList<AdjustmentSuggestion> suggestions; // 调整建议
    qreal beforeStdDev = 0;    // 优化前标准差
    qreal afterStdDev = 0;     // 优化后标准差
    qreal beforeAvg = 0;       // 优化前平均功率
    qreal afterAvg = 0;        // 优化后平均功率
    int passCountBefore = 0;   // 优化前达标数
    int passCountAfter = 0;    // 优化后达标数
    int totalAntennas = 0;     // 天线总数
    QString report;            // 文字报告
    QStringList warnings;
};

// 功率平衡优化器
class PowerBalanceOptimizer
{
public:
    PowerBalanceOptimizer();
    ~PowerBalanceOptimizer();

    // 设置场景
    void setScene(QGraphicsScene *scene) { m_scene = scene; }

    // 设置优化参数
    void setConfig(const OptimizationConfig &config) { m_config = config; }
    OptimizationConfig config() const { return m_config; }

    // 分析当前功率分布
    QList<AntennaPowerInfo> analyzeCurrentPower();

    // 执行优化
    OptimizationResult optimize();

    // 生成优化报告
    QString generateReport(const OptimizationResult &result);

    // 应用优化建议（自动调整器件）
    int applySuggestions(const OptimizationResult &result);

    // 计算标准差
    static qreal calculateStdDev(const QList<qreal> &values);

    // 计算平均值
    static qreal calculateAverage(const QList<qreal> &values);

private:
    QGraphicsScene *m_scene = nullptr;
    OptimizationConfig m_config;

    // 收集天线信息
    QList<AntennaPowerInfo> collectAntennas();

    // 模拟优化（基于当前功率分布计算最优配置）
    QList<AntennaPowerInfo> simulateOptimization(const QList<AntennaPowerInfo> &antennas);

    // 生成调整建议
    QList<AdjustmentSuggestion> generateSuggestions(const QList<AntennaPowerInfo> &before,
                                                      const QList<AntennaPowerInfo> &after);

    // 推荐功分器型号
    QString suggestSplitter(qreal requiredLoss);

    // 推荐耦合器型号
    QString suggestCoupler(qreal requiredCoupling);
};

} // namespace Zhifen

#endif // POWER_BALANCE_OPTIMIZER_H
