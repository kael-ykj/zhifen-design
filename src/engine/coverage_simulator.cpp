#include "coverage_simulator.h"
#include "../entities/caditem.h"
#include "../devices/deviceitem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QtMath>
#include <QLinearGradient>

namespace Zhifen {

CoverageSimulator::CoverageSimulator() {}
CoverageSimulator::~CoverageSimulator() {}

QString CoverageSimulator::bandName(FrequencyBand band) {
    switch (band) {
    case Band_2G: return "2G (900MHz)";
    case Band_3G: return "3G (2100MHz)";
    case Band_4G: return "4G (1800MHz)";
    case Band_5G: return "5G (3500MHz)";
    }
    return "未知";
}

qreal CoverageSimulator::bandFrequency(FrequencyBand band) {
    switch (band) {
    case Band_2G: return 900;
    case Band_3G: return 2100;
    case Band_4G: return 1800;
    case Band_5G: return 3500;
    }
    return 1800;
}

QString CoverageSimulator::wallMaterialName(WallMaterial mat) {
    switch (mat) {
    case Wall_Concrete: return "混凝土墙";
    case Wall_Brick: return "砖墙";
    case Wall_Glass: return "玻璃";
    case Wall_Elevator: return "电梯井";
    case Wall_Drywall: return "石膏板";
    }
    return "未知";
}

void CoverageSimulator::collectFromScene(QGraphicsScene *scene) {
    m_antennas.clear();
    m_walls.clear();
    if (!scene) return;

    for (auto *item : scene->items()) {
        auto *cad = dynamic_cast<CadItem*>(item);
        if (!cad) continue;

        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            DeviceType dt = dev->deviceType();
            if (dt >= DevAntennaOmni && dt <= DevAntennaGrid) {
                AntennaInfo ant;
                ant.position = dev->pos();
                ant.txPower = m_config.txPower;
                ant.gain = m_config.antennaGain;
                ant.band = m_config.band;
                ant.name = dev->deviceTypeName();
                m_antennas.append(ant);
            }
        }
        // 墙体识别：从DXF导入的线条中识别墙体（简化：暂不自动识别，后续可扩展）
    }
}

qreal CoverageSimulator::freeSpaceLoss(qreal distanceMeters, qreal frequencyMHz) {
    if (distanceMeters < 0.1) return 0;
    // FSPL = 20*log10(d) + 20*log10(f) - 27.55 (d:米, f:MHz)
    return 20.0 * qLn(distanceMeters) / qLn(10.0) + 20.0 * qLn(frequencyMHz) / qLn(10.0) - 27.55;
}

bool CoverageSimulator::lineIntersectsWall(const QPointF &p1, const QPointF &p2, const WallInfo &wall, QPointF &intersectPoint) {
    // 线段相交检测
    qreal x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
    qreal x3 = wall.start.x(), y3 = wall.start.y(), x4 = wall.end.x(), y4 = wall.end.y();

    qreal denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (qAbs(denom) < 1e-10) return false;

    qreal t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    qreal u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        intersectPoint = QPointF(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
        return true;
    }
    return false;
}

qreal CoverageSimulator::wallPenetrationLoss(const QPointF &from, const QPointF &to) {
    qreal totalLoss = 0;
    QPointF intersect;
    for (const auto &wall : m_walls) {
        if (lineIntersectsWall(from, to, wall, intersect)) {
            totalLoss += m_config.wallAttenuation.value(wall.material, 10.0);
        }
    }
    return totalLoss;
}

qreal CoverageSimulator::calculateSignal(const QPointF &point, const AntennaInfo &antenna) {
    qreal dist = qSqrt(qPow(point.x() - antenna.position.x(), 2) +
                       qPow(point.y() - antenna.position.y(), 2));
    if (dist < 0.1) return antenna.txPower + antenna.gain;

    qreal freq = bandFrequency(antenna.band);
    qreal fspl = freeSpaceLoss(dist, freq);
    qreal wallLoss = wallPenetrationLoss(antenna.position, point);

    // 对数距离路径损耗模型 (简化)
    qreal pathLoss = fspl + 10.0 * 2.0 * qLn(dist / 1.0) / qLn(10.0); // n=2.0

    return antenna.txPower + antenna.gain - pathLoss - wallLoss;
}

QColor CoverageSimulator::signalToColor(qreal signal) {
    // 信号范围: -120 ~ -40 dBm
    // 颜色渐变: 红(-120) -> 橙 -> 黄 -> 绿(-40)
    qreal normalized = qBound(0.0, (signal + 120.0) / 80.0, 1.0);

    if (normalized < 0.25) {
        // 红 -> 橙
        qreal t = normalized / 0.25;
        return QColor(255, qRound(100 * t), 0);
    } else if (normalized < 0.5) {
        // 橙 -> 黄
        qreal t = (normalized - 0.25) / 0.25;
        return QColor(255, qRound(100 + 155 * t), 0);
    } else if (normalized < 0.75) {
        // 黄 -> 浅绿
        qreal t = (normalized - 0.5) / 0.25;
        return QColor(qRound(255 - 155 * t), 255, 0);
    } else {
        // 浅绿 -> 深绿
        qreal t = (normalized - 0.75) / 0.25;
        return QColor(0, 255, qRound(100 * t));
    }
}

SimulationResult CoverageSimulator::simulate(const QRectF &area) {
    SimulationResult result;
    result.area = area;
    result.gridSize = m_config.gridSize;
    result.antennas = m_antennas;

    if (m_antennas.isEmpty()) {
        result.warnings.append("未找到天线，无法仿真");
        result.success = false;
        return result;
    }

    int cols = qCeil(area.width() / m_config.gridSize) + 1;
    int rows = qCeil(area.height() / m_config.gridSize) + 1;

    result.signalGrid = QList<QList<qreal>>();
    for (int i = 0; i < rows; i++) result.signalGrid.append(QList<qreal>());
    qreal totalSignal = 0;
    int weakCount = 0;
    int totalPoints = 0;

    for (int row = 0; row < rows; row++) {
        for (int j = 0; j < cols; j++) result.signalGrid[row].append(0.0);
        for (int col = 0; col < cols; col++) {
            QPointF point(area.left() + col * m_config.gridSize,
                          area.top() + row * m_config.gridSize);

            // 取所有天线的最强信号
            qreal bestSignal = -120.0;
            for (const auto &antenna : m_antennas) {
                qreal sig = calculateSignal(point, antenna);
                if (sig > bestSignal) bestSignal = sig;
            }

            result.signalGrid[row][col] = bestSignal;
            result.maxSignal = qMax(result.maxSignal, bestSignal);
            result.minSignal = qMin(result.minSignal, bestSignal);
            totalSignal += bestSignal;
            totalPoints++;
            if (bestSignal < m_config.weakThreshold) weakCount++;
        }
    }

    result.avgSignal = totalSignal / qMax(1, totalPoints);
    result.weakCoverageRatio = (qreal)weakCount / qMax(1, totalPoints);

    // 生成热力图
    result.heatmapImage = generateHeatmap(result, 0.5);

    result.success = true;
    return result;
}

QImage CoverageSimulator::generateHeatmap(const SimulationResult &result, qreal opacity) {
    if (result.signalGrid.isEmpty()) return QImage();

    int rows = result.signalGrid.size();
    int cols = result.signalGrid[0].size();
    int pixelSize = 4; // 每个网格点对应4x4像素

    QImage image(cols * pixelSize, rows * pixelSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            qreal signal = result.signalGrid[row][col];
            QColor color = signalToColor(signal);
            color.setAlphaF(opacity);
            painter.fillRect(col * pixelSize, row * pixelSize, pixelSize, pixelSize, color);
        }
    }

    painter.end();
    return image;
}

} // namespace Zhifen
