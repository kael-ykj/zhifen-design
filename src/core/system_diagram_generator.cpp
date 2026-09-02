#include "system_diagram_generator.h"
#include <QtMath>
#include <algorithm>

namespace Zhifen {

SystemDiagramGenerator::SystemDiagramGenerator()
    : m_mode(SystemLayoutMode::ModeA)
{
}

bool SystemDiagramGenerator::generate()
{
    if (m_network.devices.isEmpty())
        return false;

    // 1. 构建树形层次
    buildTreeLevels();

    // 2. 按模式布局
    switch (m_mode) {
        case SystemLayoutMode::ModeA: layoutModeA(); break;
        case SystemLayoutMode::ModeB: layoutModeB(); break;
        case SystemLayoutMode::ModeC: layoutModeC(); break;
        case SystemLayoutMode::ModeD: layoutModeD(); break;
    }

    // 3. 生成馈线路径
    for (auto &cable : m_network.cables) {
        DiagramCable dc;
        dc.fromDeviceId = cable.fromDeviceId;
        dc.toDeviceId = cable.toDeviceId;
        dc.length = cable.length;
        dc.loss = cable.totalLoss;
        dc.type = cable.type;

        // 找到两端器件位置
        QPointF fromPos, toPos;
        for (auto &node : m_nodes) {
            if (node.deviceId == cable.fromDeviceId) fromPos = node.position;
            if (node.deviceId == cable.toDeviceId) toPos = node.position;
        }
        dc.points = generateRightAnglePath(fromPos, toPos);
        m_cables.append(dc);
    }

    // 4. 自动编号
    autoNumbering();

    // 5. 计算功率
    calculatePower();

    return true;
}

void SystemDiagramGenerator::buildTreeLevels()
{
    // 从信源开始，BFS遍历确定层级
    if (!m_network.devices.contains(m_network.sourceDeviceId))
        return;

    QMap<QString, int> levelMap;
    QList<QString> queue;
    queue.append(m_network.sourceDeviceId);
    levelMap[m_network.sourceDeviceId] = 0;

    while (!queue.isEmpty()) {
        QString currentId = queue.takeFirst();
        if (!m_network.devices.contains(currentId)) continue;
        auto dev = m_network.devices[currentId];
        int currentLevel = levelMap[currentId];

        for (auto *port : dev->outputPorts()) {
            if (!port->connectedDeviceId.isEmpty() &&
                !levelMap.contains(port->connectedDeviceId)) {
                levelMap[port->connectedDeviceId] = currentLevel + 1;
                queue.append(port->connectedDeviceId);
            }
        }
    }

    // 创建节点
    for (auto &dev : m_network.devices) {
        DiagramNode node;
        node.deviceId = dev->id;
        node.type = dev->type;
        node.level = levelMap.value(dev->id, 0);
        node.power = dev->outputPower;
        m_nodes.append(node);
    }

    // 按层级排序
    std::sort(m_nodes.begin(), m_nodes.end(), [](const DiagramNode &a, const DiagramNode &b) {
        if (a.level != b.level) return a.level < b.level;
        return a.deviceId < b.deviceId;
    });
}

void SystemDiagramGenerator::layoutModeA()
{
    // ModeA: 信源在最下方，主干水平，器件向上排列
    // 这是迪弗/天越最常用的模式

    const double levelHeight = 80;    // 层间距
    const double deviceWidth = 70;    // 器件宽度
    const double startY = 400;        // 起始Y坐标（信源在底部）

    // 按层级分组
    QMap<int, QList<DiagramNode*>> levelGroups;
    for (auto &node : m_nodes) {
        levelGroups[node.level].append(&node);
    }

    // 每层从左到右排列
    for (auto it = levelGroups.begin(); it != levelGroups.end(); ++it) {
        int level = it.key();
        auto &group = it.value();
        double x = 80;
        double y = startY - level * levelHeight;

        for (auto *node : group) {
            node->position = QPointF(x, y);
            node->column = x / deviceWidth;
            x += deviceWidth + 30;
        }
    }
}

void SystemDiagramGenerator::layoutModeB()
{
    // ModeB: 对称布局
    layoutModeA();  // 简化实现，后续完善
}

void SystemDiagramGenerator::layoutModeC()
{
    // ModeC: 上下对称
    layoutModeA();
}

void SystemDiagramGenerator::layoutModeD()
{
    // ModeD: 全部水平
    layoutModeA();
}

QList<QPointF> SystemDiagramGenerator::generateRightAnglePath(const QPointF &from, const QPointF &to)
{
    // 生成直角转弯路径（室分系统图标准画法）
    QList<QPointF> points;
    points.append(from);

    if (qAbs(from.x() - to.x()) < 1) {
        // 垂直线
        points.append(to);
    } else if (qAbs(from.y() - to.y()) < 1) {
        // 水平线
        points.append(to);
    } else {
        // L形直角：先水平后垂直
        double midX = (from.x() + to.x()) / 2;
        points.append(QPointF(midX, from.y()));
        points.append(QPointF(midX, to.y()));
        points.append(to);
    }

    return points;
}

double SystemDiagramGenerator::calculateCableLoss(double length, const QString &type)
{
    // 馈线单位损耗（dB/m）
    // 1/2馈线：约0.07 dB/m（900MHz），0.11 dB/m（2000MHz）
    // 7/8馈线：约0.04 dB/m（900MHz），0.06 dB/m（2000MHz）
    // 跳线：约0.1 dB/m
    double lossPerMeter = 0.07;
    if (type.contains("7/8") || type.contains("5/4"))
        lossPerMeter = 0.04;
    else if (type.contains("跳线") || type.contains("jumper"))
        lossPerMeter = 0.1;
    else if (type.contains("漏缆"))
        lossPerMeter = 0.05;

    return length * lossPerMeter;
}

void SystemDiagramGenerator::autoNumbering(const QString &floor)
{
    int couplerCount = 0;
    int splitterCount = 0;
    int antennaCount = 0;

    for (auto &node : m_nodes) {
        if (node.type == DeviceType::Coupler) {
            couplerCount++;
            node.label = QString("T%1-%2").arg(couplerCount).arg(floor);
        } else if (node.type == DeviceType::Splitter) {
            splitterCount++;
            node.label = QString("PS%1-%2").arg(splitterCount).arg(floor);
        } else if (node.type == DeviceType::Antenna) {
            antennaCount++;
            node.label = QString("ANT%1-%2").arg(antennaCount).arg(floor);
        } else if (node.type == DeviceType::Source) {
            node.label = "信源";
        }
    }
}

void SystemDiagramGenerator::calculatePower()
{
    // 调用拓扑网络的正向功率计算
    m_network.calculatePowerForward();

    // 更新节点功率
    for (auto &node : m_nodes) {
        if (m_network.devices.contains(node.deviceId)) {
            node.power = m_network.devices[node.deviceId]->outputPower;
        }
    }
}

QMap<QString, int> SystemDiagramGenerator::materialSummary() const
{
    return m_network.materialCount();
}

void SystemDiagramGenerator::sortDevicesByLevel()
{
    std::sort(m_nodes.begin(), m_nodes.end(), [](const DiagramNode &a, const DiagramNode &b) {
        if (a.level != b.level) return a.level < b.level;
        return a.position.x() < b.position.x();
    });
}

} // namespace Zhifen
