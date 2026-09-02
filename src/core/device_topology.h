#ifndef DEVICE_TOPOLOGY_H
#define DEVICE_TOPOLOGY_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <memory>

namespace Zhifen {

// 器件端口类型
enum class PortType {
    Input,          // 输入端口
    OutputThrough,  // 直通输出（耦合器）
    OutputCoupled,  // 耦合输出（耦合器）
    OutputSplit,    // 分配输出（功分器）
    AntennaPort,    // 天线端口
    SourcePort      // 信源输出端口
};

// 器件类型
enum class DeviceType {
    Source,         // 信源（BBU/RRU/直放站）
    Coupler,        // 耦合器
    Splitter,       // 功分器
    Antenna,        // 天线
    Amplifier,      // 放大器
    Load,           // 负载
    Attenuator,     // 衰减器
    Combiner,       // 合路器
    POI,            // POI合路平台
    Junction        // 连接点
};

// 端口
struct DevicePort {
    QString id;                  // 端口ID
    PortType type;               // 端口类型
    QPointF position;            // 端口位置（图纸坐标）
    QString connectedDeviceId;   // 连接的器件ID
    QString connectedPortId;     // 连接的端口ID
    double cableLength;          // 连接馈线长度（米）
    double cableLoss;            // 馈线损耗（dB）
    QString cableType;           // 馈线类型（1/2, 7/8, 跳线）

    DevicePort() : type(PortType::Input), cableLength(0), cableLoss(0) {}
};

// 器件基类
class DeviceNode {
public:
    QString id;                  // 器件ID（如 T1-5F, ANT1-5F, PS1-5F）
    DeviceType type;             // 器件类型
    QString name;                // 器件名称
    QPointF position;            // 位置
    QString floor;               // 楼层
    int layer;                   // 系统图层级（0=信源层）

    QList<DevicePort> ports;     // 端口列表

    // 功率相关
    double inputPower;           // 输入功率（dBm）
    double outputPower;          // 输出功率（dBm）
    double targetPower;          // 目标功率（天线用）

    // 器件参数
    int couplerDb;               // 耦合器dB值
    int splitterWays;            // 功分器路数
    double insertionLoss;        // 插损（dB）
    double gain;                 // 增益（放大器用）

    DeviceNode() : type(DeviceType::Junction), layer(0), inputPower(0), outputPower(0),
                   targetPower(0), couplerDb(0), splitterWays(2), insertionLoss(0), gain(0) {}

    // 获取输入端口
    DevicePort* inputPort() {
        for (auto &p : ports) {
            if (p.type == PortType::Input || p.type == PortType::SourcePort)
                return &p;
        }
        return nullptr;
    }

    // 获取输出端口列表
    QList<DevicePort*> outputPorts() {
        QList<DevicePort*> list;
        for (auto &p : ports) {
            if (p.type == PortType::OutputThrough ||
                p.type == PortType::OutputCoupled ||
                p.type == PortType::OutputSplit ||
                p.type == PortType::AntennaPort)
                list.append(&p);
        }
        return list;
    }
};

// 馈线连接
class CableLink {
public:
    QString id;
    QString fromDeviceId;
    QString fromPortId;
    QString toDeviceId;
    QString toPortId;
    double length;               // 长度（米）
    QString type;                // 类型（1/2, 7/8, 跳线, 漏缆）
    double lossPerMeter;         // 每米损耗（dB/m）
    double totalLoss;            // 总损耗（dB）

    CableLink() : length(0), lossPerMeter(0.07), totalLoss(0) {}

    void calculateLoss() {
        totalLoss = length * lossPerMeter;
    }
};

// 拓扑网络（整个室分系统的连接关系）
class DistributionNetwork {
public:
    QMap<QString, std::shared_ptr<DeviceNode>> devices;  // 所有器件
    QMap<QString, CableLink> cables;                     // 所有馈线
    QString sourceDeviceId;                              // 信源器件ID

    // 添加器件
    void addDevice(std::shared_ptr<DeviceNode> dev) {
        devices[dev->id] = dev;
    }

    // 添加馈线连接
    void addCable(const CableLink &cable) {
        cables[cable.id] = cable;
        // 更新器件端口连接关系
        if (devices.contains(cable.fromDeviceId)) {
            for (auto &p : devices[cable.fromDeviceId]->ports) {
                if (p.id == cable.fromPortId) {
                    p.connectedDeviceId = cable.toDeviceId;
                    p.connectedPortId = cable.toPortId;
                    p.cableLength = cable.length;
                    p.cableLoss = cable.totalLoss;
                    p.cableType = cable.type;
                }
            }
        }
        if (devices.contains(cable.toDeviceId)) {
            for (auto &p : devices[cable.toDeviceId]->ports) {
                if (p.id == cable.toPortId) {
                    p.connectedDeviceId = cable.fromDeviceId;
                    p.connectedPortId = cable.fromPortId;
                    p.cableLength = cable.length;
                    p.cableLoss = cable.totalLoss;
                    p.cableType = cable.type;
                }
            }
        }
    }

    // 正向功率计算（从信源到天线）
    void calculatePowerForward() {
        if (!devices.contains(sourceDeviceId)) return;
        auto source = devices[sourceDeviceId];
        calculateNodePower(source.get(), source->outputPower);
    }

    // 逆向功率反推（从天线目标功率反推耦合器和信源）
    void calculatePowerBackward() {
        // 先找到所有天线
        QList<std::shared_ptr<DeviceNode>> antennas;
        for (auto &dev : devices) {
            if (dev->type == DeviceType::Antenna)
                antennas.append(dev);
        }
        // 从每个天线向上反推
        for (auto &ant : antennas) {
            backtrackNodePower(ant.get());
        }
    }

    // 获取所有天线
    QList<std::shared_ptr<DeviceNode>> antennas() const {
        QList<std::shared_ptr<DeviceNode>> list;
        for (auto &dev : devices) {
            if (dev->type == DeviceType::Antenna)
                list.append(dev);
        }
        return list;
    }

    // 材料统计
    QMap<QString, int> materialCount() const {
        QMap<QString, int> count;
        for (auto &dev : devices) {
            QString key = deviceTypeString(dev->type);
            if (dev->type == DeviceType::Coupler)
                key = QString("%1dB耦合器").arg(dev->couplerDb);
            else if (dev->type == DeviceType::Splitter)
                key = QString("%1功分器").arg(dev->splitterWays);
            count[key]++;
        }
        // 统计馈线总长度
        QMap<QString, double> cableLength;
        for (auto &cable : cables) {
            cableLength[cable.type] += cable.length;
        }
        for (auto it = cableLength.begin(); it != cableLength.end(); ++it) {
            count[QString("%1馈线(米)").arg(it.key())] = it.value();
        }
        return count;
    }

private:
    // 递归计算节点功率（正向）
    void calculateNodePower(DeviceNode *node, double inputPower) {
        if (!node) return;
        node->inputPower = inputPower;

        if (node->type == DeviceType::Antenna) {
            node->outputPower = inputPower;
            return;
        }

        // 计算每个输出端口的功率
        for (auto *port : node->outputPorts()) {
            double outPower = inputPower - node->insertionLoss;

            if (node->type == DeviceType::Coupler) {
                if (port->type == PortType::OutputCoupled) {
                    outPower -= node->couplerDb;  // 耦合端损耗=耦合度
                } else if (port->type == PortType::OutputThrough) {
                    // 直通端插损（典型值）
                    double throughLoss = 0.3;
                    if (node->couplerDb <= 7) throughLoss = 1.3;
                    else if (node->couplerDb <= 10) throughLoss = 0.8;
                    else if (node->couplerDb <= 15) throughLoss = 0.5;
                    outPower -= throughLoss;
                }
            } else if (node->type == DeviceType::Splitter) {
                // 功分器分配损耗 = 10*log10(ways) + 插损
                double splitLoss = 10 * log10(node->splitterWays) + 0.2;
                outPower -= splitLoss;
            } else if (node->type == DeviceType::Amplifier) {
                outPower += node->gain;
            }

            // 减去馈线损耗
            outPower -= port->cableLoss;

            // 递归计算下一级
            if (devices.contains(port->connectedDeviceId)) {
                calculateNodePower(devices[port->connectedDeviceId].get(), outPower);
            }
        }
    }

    // 逆向反推功率
    void backtrackNodePower(DeviceNode *node) {
        // 简化版：从天线目标功率向上反推
        // 实际实现需要更复杂的算法
        Q_UNUSED(node);
    }

    QString deviceTypeString(DeviceType type) const {
        switch (type) {
            case DeviceType::Source: return "信源";
            case DeviceType::Coupler: return "耦合器";
            case DeviceType::Splitter: return "功分器";
            case DeviceType::Antenna: return "天线";
            case DeviceType::Amplifier: return "放大器";
            case DeviceType::Load: return "负载";
            case DeviceType::Attenuator: return "衰减器";
            case DeviceType::Combiner: return "合路器";
            case DeviceType::POI: return "POI";
            default: return "其他";
        }
    }
};

} // namespace Zhifen

#endif // DEVICE_TOPOLOGY_H
