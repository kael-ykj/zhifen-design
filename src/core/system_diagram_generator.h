#ifndef SYSTEM_DIAGRAM_GENERATOR_H
#define SYSTEM_DIAGRAM_GENERATOR_H

#include "device_topology.h"
#include <QList>
#include <QPointF>
#include <QString>

namespace Zhifen {

// 系统图布局模式（对标天越4种模式）
enum class SystemLayoutMode {
    ModeA,  // 功分器下方输出口馈线水平，其余向上
    ModeB,  // 功分器各输出口馈线对称
    ModeC,  // 功分器输出口馈线直接向上或向下对称
    ModeD   // 功分器输出口馈线水平
};

// 系统图中的节点位置
struct DiagramNode {
    QString deviceId;
    DeviceType type;
    QPointF position;       // 在系统图中的位置
    int level;              // 层级（0=信源层）
    int column;             // 列位置
    QString label;          // 标注文字
    double power;           // 功率标注
};

// 系统图中的馈线
struct DiagramCable {
    QString fromDeviceId;
    QString toDeviceId;
    QList<QPointF> points;  // 馈线路径点（直角转弯）
    double length;
    double loss;
    QString type;
};

// 系统图生成器
// 核心原理：从拓扑网络出发，按树形结构层次排列器件，
// 馈线走直角，自动计算长度和损耗，对标天越/迪弗系统图风格
class SystemDiagramGenerator
{
public:
    SystemDiagramGenerator();

    // 设置拓扑网络
    void setNetwork(const DistributionNetwork &network) { m_network = network; }

    // 设置布局模式
    void setLayoutMode(SystemLayoutMode mode) { m_mode = mode; }

    // 生成系统图
    bool generate();

    // 获取生成结果
    QList<DiagramNode> nodes() const { return m_nodes; }
    QList<DiagramCable> cables() const { return m_cables; }

    // 自动编号（对标天越：T=耦合器，PS=功分器，ANT=天线）
    void autoNumbering(const QString &floor = "5F");

    // 自动计算功率
    void calculatePower();

    // 材料统计
    QMap<QString, int> materialSummary() const;

private:
    DistributionNetwork m_network;
    SystemLayoutMode m_mode;
    QList<DiagramNode> m_nodes;
    QList<DiagramCable> m_cables;

    // 构建树形层次结构
    void buildTreeLevels();

    // 按模式布局
    void layoutModeA();  // 功分器下方输出水平，其余向上
    void layoutModeB();  // 对称布局
    void layoutModeC();  // 上下对称
    void layoutModeD();  // 全部水平

    // 生成直角馈线路径
    QList<QPointF> generateRightAnglePath(const QPointF &from, const QPointF &to);

    // 计算馈线损耗
    double calculateCableLoss(double length, const QString &type);

    // 器件位置排序
    void sortDevicesByLevel();
};

} // namespace Zhifen

#endif // SYSTEM_DIAGRAM_GENERATOR_H
