#include "system_diagram_generator.h"
#include "../entities/caditem.h"
#include "../devices/deviceitem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QFont>
#include <QtMath>

namespace Zhifen {

SystemDiagramGenerator::SystemDiagramGenerator() {}

SystemDiagramGenerator::~SystemDiagramGenerator() {
    clear();
}

void SystemDiagramGenerator::clear() {
    qDeleteAll(m_allNodes);
    m_allNodes.clear();
}

QString SystemDiagramGenerator::classifyDevice(const QString &entityType) {
    if (entityType.contains("信源") || entityType.contains("RRU") || entityType.contains("BBU") ||
        entityType.contains("微基站") || entityType.contains("直放站") || entityType.contains("干放") ||
        entityType.contains("pRRU")) return "信源";
    if (entityType.contains("功分")) return "功分器";
    if (entityType.contains("耦合")) return "耦合器";
    if (entityType.contains("合路") || entityType.contains("电桥")) return "合路器";
    if (entityType.contains("天线")) return "天线";
    return "其他";
}

int SystemDiagramGenerator::outputPortCount(const QString &type) {
    if (type == "功分器") return 2;  // 默认二功分，实际可根据型号
    if (type == "耦合器") return 2;  // 直通+耦合
    if (type == "合路器") return 1;
    if (type == "信源") return 1;
    return 0;
}

TopoNode* SystemDiagramGenerator::createNode(const QString &type, const QString &name, int level) {
    TopoNode *node = new TopoNode();
    node->id = QString("node_%1").arg(m_allNodes.size());
    node->type = type;
    node->name = name;
    node->level = level;
    m_allNodes.append(node);
    return node;
}

void SystemDiagramGenerator::parseTopology(QGraphicsScene *scene, SystemDiagramResult &result) {
    if (!scene) {
        result.errors.append("场景为空");
        return;
    }

    // 收集所有器件
    QList<TopoNode*> sources, splitters, couplers, combiners, antennas;

    for (auto *item : scene->items()) {
        auto *cad = dynamic_cast<CadItem*>(item);
        if (!cad) continue;
        QString etype = cad->entityType();
        QString cls = classifyDevice(etype);

        if (cls == "信源") {
            auto *dev = dynamic_cast<DeviceItem*>(item);
            QString name = dev ? dev->deviceTypeName() : etype;
            sources.append(createNode("信源", name, 0));
        } else if (cls == "功分器") {
            auto *dev = dynamic_cast<DeviceItem*>(item);
            QString name = dev ? dev->deviceTypeName() : etype;
            splitters.append(createNode("功分器", name, 1));
        } else if (cls == "耦合器") {
            auto *dev = dynamic_cast<DeviceItem*>(item);
            QString name = dev ? dev->deviceTypeName() : etype;
            couplers.append(createNode("耦合器", name, 1));
        } else if (cls == "合路器") {
            auto *dev = dynamic_cast<DeviceItem*>(item);
            QString name = dev ? dev->deviceTypeName() : etype;
            combiners.append(createNode("合路器", name, 1));
        } else if (cls == "天线") {
            auto *dev = dynamic_cast<DeviceItem*>(item);
            QString name = dev ? dev->deviceTypeName() : etype;
            antennas.append(createNode("天线", name, 2));
        }
    }

    if (sources.isEmpty()) {
        result.errors.append("未找到信源器件，无法生成系统图");
        return;
    }

    // 构建拓扑：信源 -> 功分器/耦合器 -> 天线
    // 简化策略：按数量平均分配
    TopoNode *root = sources.first();

    // 中间层器件（功分器+耦合器+合路器）
    QList<TopoNode*> middleDevices;
    middleDevices.append(splitters);
    middleDevices.append(couplers);
    middleDevices.append(combiners);

    if (middleDevices.isEmpty() && antennas.isEmpty()) {
        result.errors.append("未找到功分器/耦合器/天线，无法生成系统图");
        return;
    }

    // 如果没有中间层，信源直接连天线
    if (middleDevices.isEmpty()) {
        for (auto *ant : antennas) {
            ant->level = 1;
            ant->parent = root;
            root->children.append(ant);
        }
    } else {
        // 信源连接中间层
        for (auto *mid : middleDevices) {
            mid->level = 1;
            mid->parent = root;
            root->children.append(mid);
        }

        // 中间层连接天线（平均分配）
        if (!antennas.isEmpty()) {
            int perDevice = qMax(1, antennas.size() / middleDevices.size());
            int idx = 0;
            for (int i = 0; i < middleDevices.size(); i++) {
                int count = (i == middleDevices.size() - 1) ? (antennas.size() - idx) : perDevice;
                for (int j = 0; j < count && idx < antennas.size(); j++) {
                    antennas[idx]->level = 2;
                    antennas[idx]->parent = middleDevices[i];
                    middleDevices[i]->children.append(antennas[idx]);
                    idx++;
                }
            }
        }
    }

    // 收集所有节点
    result.nodes = m_allNodes;

    // 构建连接
    for (auto *node : m_allNodes) {
        for (auto *child : node->children) {
            TopoConnection conn;
            conn.from = node;
            conn.to = child;
            conn.loss = 1.0;  // 默认1dB馈线损耗
            conn.length = 5.0; // 默认5米
            result.connections.append(conn);
        }
    }
}

void SystemDiagramGenerator::layout(SystemDiagramResult &result) {
    if (result.nodes.isEmpty()) return;

    // 按层级分组
    QMap<int, QList<TopoNode*>> levels;
    for (auto *node : result.nodes) {
        levels[node->level].append(node);
    }

    qreal vGap = 120;  // 垂直间距
    qreal hGap = 100;  // 水平间距
    qreal nodeWidth = 80;

    int maxLevel = levels.keys().last();

    for (int level = 0; level <= maxLevel; level++) {
        if (!levels.contains(level)) continue;
        auto &nodes = levels[level];
        qreal totalWidth = nodes.size() * nodeWidth + (nodes.size() - 1) * hGap;
        qreal startX = -totalWidth / 2;
        qreal y = level * vGap;

        for (int i = 0; i < nodes.size(); i++) {
            nodes[i]->pos = QPointF(startX + i * (nodeWidth + hGap) + nodeWidth/2, y);
        }
    }

    // 计算边界
    qreal minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (auto *node : result.nodes) {
        minX = qMin(minX, node->pos.x() - nodeWidth/2);
        maxX = qMax(maxX, node->pos.x() + nodeWidth/2);
        minY = qMin(minY, node->pos.y() - 30);
        maxY = qMax(maxY, node->pos.y() + 30);
    }
    result.boundingRect = QRectF(minX - 20, minY - 20, maxX - minX + 40, maxY - minY + 40);
}

void SystemDiagramGenerator::calculatePower(SystemDiagramResult &result) {
    // 精确链路预算：根据器件类型计算损耗
    // 信源输出功率：宏基站43dBm，微基站33dBm，RRU37dBm
    for (auto *node : result.nodes) {
        if (node->level == 0) {
            // 根据信源类型设置输出功率
            if (node->name.contains("宏基站") || node->name.contains("BBU")) {
                node->outputPower = 43.0;
            } else if (node->name.contains("微基站") || node->name.contains("RRU")) {
                node->outputPower = 37.0;
            } else if (node->name.contains("直放站") || node->name.contains("干放")) {
                node->outputPower = 33.0;
            } else if (node->name.contains("皮基站") || node->name.contains("pRRU")) {
                node->outputPower = 24.0;
            } else {
                node->outputPower = 43.0;  // 默认43dBm
            }
        } else if (node->parent) {
            // 馈线损耗：根据连接长度计算（1/2馈线0.07dB/m）
            qreal feederLoss = 0.0;
            for (const auto &conn : result.connections) {
                if (conn.from == node->parent && conn.to == node) {
                    feederLoss = conn.length * 0.07;  // 1/2馈线损耗
                    conn.loss = feederLoss;
                    break;
                }
            }
            node->inputPower = node->parent->outputPower - feederLoss;

            // 器件插入损耗
            if (node->type == "功分器") {
                // 根据功分器端口数计算分配损耗
                if (node->name.contains("二功分") || node->name.contains("2功分")) {
                    node->loss = 3.5;  // 分配3dB + 插入0.5dB
                } else if (node->name.contains("三功分") || node->name.contains("3功分")) {
                    node->loss = 5.2;  // 分配4.8dB + 插入0.4dB
                } else if (node->name.contains("四功分") || node->name.contains("4功分")) {
                    node->loss = 6.5;  // 分配6dB + 插入0.5dB
                } else {
                    node->loss = 3.5;  // 默认二功分
                }
                node->outputPower = node->inputPower - node->loss;
            } else if (node->type == "耦合器") {
                // 耦合器直通损耗0.5dB，耦合端根据耦合度
                node->loss = 0.5;  // 直通插入损耗
                node->outputPower = node->inputPower - node->loss;
            } else if (node->type == "合路器") {
                node->loss = 0.5;  // 插入损耗
                node->outputPower = node->inputPower - node->loss;
            } else if (node->type == "天线") {
                node->loss = 0.0;
                node->outputPower = node->inputPower;  // 天线输入功率
            } else {
                node->outputPower = node->inputPower;
            }
        }
    }
}

void SystemDiagramGenerator::assignDeviceIds(SystemDiagramResult &result) {
    int antIdx = 1, splIdx = 1, cplIdx = 1, srcIdx = 1, combIdx = 1;
    for (auto *node : result.nodes) {
        if (node->type == "信源") node->deviceId = QString("SRC-%1").arg(srcIdx++, 3, 10, QChar('0'));
        else if (node->type == "功分器") node->deviceId = QString("SPL-%1").arg(splIdx++, 3, 10, QChar('0'));
        else if (node->type == "耦合器") node->deviceId = QString("CPL-%1").arg(cplIdx++, 3, 10, QChar('0'));
        else if (node->type == "合路器") node->deviceId = QString("CMB-%1").arg(combIdx++, 3, 10, QChar('0'));
        else if (node->type == "天线") node->deviceId = QString("ANT-%1").arg(antIdx++, 3, 10, QChar('0'));
    }
}

SystemDiagramResult SystemDiagramGenerator::generate(QGraphicsScene *scene, SystemDiagramMode mode) {
    clear();
    SystemDiagramResult result;

    // 1. 解析拓扑
    parseTopology(scene, result);
    if (!result.errors.isEmpty() && result.nodes.isEmpty()) {
        result.success = false;
        return result;
    }

    // 2. 布局
    layout(result);

    // 3. 正式模式：计算电平+分配编号
    if (mode == SDM_Formal) {
        calculatePower(result);
        assignDeviceIds(result);
    }

    result.success = true;
    return result;
}

void SystemDiagramGenerator::renderToScene(const SystemDiagramResult &result, QGraphicsScene *targetScene, SystemDiagramMode mode) {
    if (!targetScene) return;
    targetScene->clear();

    QPen linePen(QColor(100, 100, 100), 1.5);
    QFont labelFont("Arial", 8);
    QFont powerFont("Arial", 7);

    // 绘制连接线
    for (const auto &conn : result.connections) {
        if (!conn.from || !conn.to) continue;
        QGraphicsLineItem *line = targetScene->addLine(
            QLineF(conn.from->pos, conn.to->pos), linePen);

        // 正式模式：标注馈线损耗
        if (mode == SDM_Formal) {
            QPointF mid = (conn.from->pos + conn.to->pos) / 2;
            QGraphicsSimpleTextItem *lossText = targetScene->addSimpleText(
                QString("%1dB").arg(conn.loss, 0, 'f', 1), powerFont);
            lossText->setPos(mid + QPointF(5, -10));
            lossText->setBrush(QColor(80, 80, 80));
        }
    }

    // 绘制器件节点
    for (auto *node : result.nodes) {
        QRectF nodeRect(node->pos.x() - 40, node->pos.y() - 20, 80, 40);

        // 根据类型选择颜色
        QColor fillColor;
        if (node->type == "信源") fillColor = QColor(255, 200, 100);
        else if (node->type == "功分器") fillColor = QColor(150, 200, 255);
        else if (node->type == "耦合器") fillColor = QColor(200, 150, 255);
        else if (node->type == "合路器") fillColor = QColor(255, 150, 200);
        else if (node->type == "天线") fillColor = QColor(150, 255, 150);
        else fillColor = QColor(200, 200, 200);

        QGraphicsRectItem *rect = targetScene->addRect(nodeRect, QPen(QColor(60, 60, 60), 1.5), QBrush(fillColor));

        // 器件名称
        QGraphicsSimpleTextItem *nameText = targetScene->addSimpleText(node->name, labelFont);
        nameText->setPos(node->pos.x() - 35, node->pos.y() - 12);
        nameText->setBrush(QColor(0, 0, 0));

        // 正式模式：标注编号+电平
        if (mode == SDM_Formal) {
            // 编号
            QGraphicsSimpleTextItem *idText = targetScene->addSimpleText(node->deviceId, powerFont);
            idText->setPos(node->pos.x() - 35, node->pos.y() + 2);
            idText->setBrush(QColor(0, 0, 128));

            // 输出电平（低于10dBm红色告警）
            QColor powerColor = (node->outputPower < 10.0 && node->type == "天线") ? QColor(255, 0, 0) : QColor(180, 0, 0);
            QGraphicsSimpleTextItem *powerText = targetScene->addSimpleText(
                QString("%1dBm").arg(node->outputPower, 0, 'f', 1), powerFont);
            powerText->setPos(node->pos.x() - 35, node->pos.y() + 14);
            powerText->setBrush(powerColor);

            // 耦合器额外标注耦合端功率
            if (node->type == "耦合器") {
                qreal coupledPower = node->inputPower - 10.0;  // 默认10dB耦合度
                QGraphicsSimpleTextItem *cplText = targetScene->addSimpleText(
                    QString("耦合:%1dBm").arg(coupledPower, 0, 'f', 1), powerFont);
                cplText->setPos(node->pos.x() + 45, node->pos.y() - 5);
                cplText->setBrush(QColor(0, 100, 0));
            }
        }
    }

    // 标题
    QGraphicsSimpleTextItem *title = targetScene->addSimpleText(
        mode == SDM_Formal ? "系统图（正式模式）" : "系统图（草图模式）",
        QFont("Arial", 12, QFont::Bold));
    title->setPos(result.boundingRect.left(), result.boundingRect.top() - 30);
    title->setBrush(QColor(0, 0, 0));
}

} // namespace Zhifen
