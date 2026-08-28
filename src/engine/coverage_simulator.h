#ifndef COVERAGE_SIMULATOR_H
#define COVERAGE_SIMULATOR_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QImage>
#include <QColor>
#include <QGraphicsScene>

namespace Zhifen {

// 频段类型
enum FrequencyBand {
    Band_2G = 0,    // 900MHz
    Band_3G = 1,    // 2100MHz
    Band_4G = 2,    // 1800MHz / 2600MHz
    Band_5G = 3     // 3500MHz / 4900MHz
};

// 墙体材质
enum WallMaterial {
    Wall_Concrete = 0,   // 混凝土墙
    Wall_Brick = 1,      // 砖墙
    Wall_Glass = 2,      // 玻璃
    Wall_Elevator = 3,   // 电梯井
    Wall_Drywall = 4     // 石膏板
};

// 墙体信息
struct WallInfo {
    QPointF start;
    QPointF end;
    WallMaterial material = Wall_Concrete;
    qreal thickness = 240;  // mm
};

// 天线信息
struct AntennaInfo {
    QPointF position;
    qreal txPower = 15.0;      // dBm
    qreal gain = 2.0;          // dBi
    FrequencyBand band = Band_4G;
    QString name;
};

// 仿真参数
struct SimulationConfig {
    FrequencyBand band = Band_4G;
    qreal gridSize = 0.5;       // 米
    qreal txPower = 15.0;       // dBm (默认)
    qreal antennaGain = 2.0;    // dBi
    qreal weakThreshold = -95;  // dBm 弱覆盖阈值
    // 墙体衰减 (dB)
    QMap<WallMaterial, qreal> wallAttenuation;

    SimulationConfig() {
        wallAttenuation[Wall_Concrete] = 15.0;
        wallAttenuation[Wall_Brick] = 8.0;
        wallAttenuation[Wall_Glass] = 3.0;
        wallAttenuation[Wall_Elevator] = 25.0;
        wallAttenuation[Wall_Drywall] = 2.0;
    }
};

// 仿真结果
struct SimulationResult {
    bool success = false;
    QRectF area;                    // 仿真区域
    qreal gridSize = 0.5;           // 网格大小
    QList<QList<qreal>> signalGrid; // 信号强度网格 [row][col]
    QList<AntennaInfo> antennas;    // 天线列表
    qreal maxSignal = -100;         // 最大信号
    qreal minSignal = -120;         // 最小信号
    qreal avgSignal = -100;         // 平均信号
    qreal weakCoverageRatio = 0;    // 弱覆盖比例
    QImage heatmapImage;            // 热力图
    QStringList warnings;
};

// 覆盖仿真器
class CoverageSimulator
{
public:
    CoverageSimulator();
    ~CoverageSimulator();

    // 从场景收集天线和墙体
    void collectFromScene(QGraphicsScene *scene);

    // 设置仿真参数
    void setConfig(const SimulationConfig &config) { m_config = config; }
    SimulationConfig config() const { return m_config; }

    // 执行仿真
    SimulationResult simulate(const QRectF &area);

    // 生成热力图
    QImage generateHeatmap(const SimulationResult &result, qreal opacity = 0.5);

    // 获取频段名称
    static QString bandName(FrequencyBand band);

    // 获取频段频率 (MHz)
    static qreal bandFrequency(FrequencyBand band);

    // 获取墙体材质名称
    static QString wallMaterialName(WallMaterial mat);

private:
    SimulationConfig m_config;
    QList<AntennaInfo> m_antennas;
    QList<WallInfo> m_walls;

    // 计算某点的信号强度
    qreal calculateSignal(const QPointF &point, const AntennaInfo &antenna);

    // 计算自由空间损耗
    qreal freeSpaceLoss(qreal distanceMeters, qreal frequencyMHz);

    // 计算墙体穿透损耗
    qreal wallPenetrationLoss(const QPointF &from, const QPointF &to);

    // 判断线段与墙体相交
    bool lineIntersectsWall(const QPointF &p1, const QPointF &p2, const WallInfo &wall, QPointF &intersectPoint);

    // 信号强度转颜色
    QColor signalToColor(qreal signal);
};

} // namespace Zhifen

#endif // COVERAGE_SIMULATOR_H
