#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "mode_control/mode_control_layer.h"
#include "building_model_engine.h"

namespace zf {

// 仿真参数
struct SimulationConfig {
    double frequency_MHz{900.0};
    double gridResolution_m{0.5};  // 网格分辨率
    double maxDistance_m{50.0};    // 最大仿真距离
    double txPower_dBm{20.0};      // 发射功率
    double antennaGain_dBi{3.0};   // 天线增益
    double receiverHeight_m{1.5};  // 接收高度
    double txHeight_m{2.5};        // 发射高度
    bool includeWallLoss{true};    // 是否包含墙体损耗
};

// 热力图网格点
struct HeatmapPoint {
    Point2D position;
    double rsrp_dBm{-140.0};  // 参考信号接收功率
    int wallCount{0};         // 穿过墙体数
    double distance_m{0.0};   // 距离
};

// 热力图数据
struct HeatmapData {
    std::string floorId;
    double gridResolution_m{0.5};
    Rect2D bounds;
    int gridWidth{0};
    int gridHeight{0};
    std::vector<HeatmapPoint> points;

    // 统计
    double maxRSRP{-140.0};
    double minRSRP{-140.0};
    double avgRSRP{-140.0};
    double coverageRate{0.0};  // 覆盖率（>= -100dBm的比例）
    int weakCoverageCount{0};  // 弱覆盖点数
};

// 传播引擎
class PropagationEngine {
public:
    PropagationEngine() = default;

    void setModeManager(ModeManager* mgr) { m_modeMgr = mgr; }
    void setConfig(const SimulationConfig& cfg) { m_config = cfg; }

    // 自由空间路径损耗
    double freeSpaceLoss(double distance_m, double freq_MHz) const {
        if (distance_m < 1.0) distance_m = 1.0;
        return 20.0 * std::log10(distance_m) + 20.0 * std::log10(freq_MHz) + 32.44;
    }

    // 计算两点之间穿过的墙体数量和总损耗
    double calcWallLoss(const Point2D& from, const Point2D& to,
                        const Floor* floor, int& outWallCount) const {
        outWallCount = 0;
        if (!floor || !m_config.includeWallLoss) return 0.0;

        double totalLoss = 0.0;
        for (const auto& wall : floor->walls) {
            for (size_t i = 0; i + 1 < wall.points.size(); i++) {
                if (segmentsIntersect(from, to, wall.points[i], wall.points[i+1])) {
                    totalLoss += wall.attenuation_dB;
                    outWallCount++;
                }
            }
        }
        return totalLoss;
    }

    // 计算单点接收功率
    double calcRSRP(const Point2D& txPos, const Point2D& rxPos,
                    const Floor* floor, int& outWallCount) const {
        double dx = rxPos.x - txPos.x;
        double dy = rxPos.y - txPos.y;
        double dist_m = std::sqrt(dx*dx + dy*dy) / 1000.0; // 假设坐标毫米转米

        if (dist_m > m_config.maxDistance_m) {
            outWallCount = 0;
            return -140.0;
        }

        double pl = freeSpaceLoss(dist_m, m_config.frequency_MHz);
        double wallLoss = calcWallLoss(txPos, rxPos, floor, outWallCount);

        // 高度差附加损耗
        double heightDiff = std::abs(m_config.txHeight_m - m_config.receiverHeight_m);
        double heightFactor = 1.0;
        if (heightDiff > 0) {
            double dist3d = std::sqrt(dist_m * dist_m + heightDiff * heightDiff);
            pl = freeSpaceLoss(dist3d, m_config.frequency_MHz);
        }

        double rsrp = m_config.txPower_dBm + m_config.antennaGain_dBi - pl - wallLoss;
        return std::max(rsrp, -140.0);
    }

    // 生成楼层热力图
    int generateHeatmap(const Floor* floor, const Point2D& txPosition,
                        HeatmapData& outHeatmap) {
        if (!floor) return ZF_ERR_ARG;

        // 草图模式限制
        if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE) {
            return m_modeMgr->checkHeavyComputePermission();
        }

        // 计算楼层边界
        Rect2D bounds = calcFloorBounds(floor);
        if (bounds.size.w <= 0 || bounds.size.h <= 0) {
            return ZF_ERR_ARG;
        }

        outHeatmap.floorId = floor->floorId;
        outHeatmap.gridResolution_m = m_config.gridResolution_m;
        outHeatmap.bounds = bounds;

        double res_px = m_config.gridResolution_m * 1000.0; // 米转毫米
        outHeatmap.gridWidth = (int)(bounds.size.w / res_px) + 1;
        outHeatmap.gridHeight = (int)(bounds.size.h / res_px) + 1;
        outHeatmap.points.clear();
        outHeatmap.points.reserve(outHeatmap.gridWidth * outHeatmap.gridHeight);

        double sumRSRP = 0;
        int validCount = 0;

        for (int gy = 0; gy < outHeatmap.gridHeight; gy++) {
            for (int gx = 0; gx < outHeatmap.gridWidth; gx++) {
                HeatmapPoint hp;
                hp.position.x = bounds.origin.x + gx * res_px;
                hp.position.y = bounds.origin.y + gy * res_px;

                int wallCount = 0;
                hp.rsrp_dBm = calcRSRP(txPosition, hp.position, floor, wallCount);
                hp.wallCount = wallCount;
                hp.distance_m = std::sqrt(
                    std::pow(hp.position.x - txPosition.x, 2) +
                    std::pow(hp.position.y - txPosition.y, 2)) / 1000.0;

                outHeatmap.points.push_back(hp);

                if (hp.rsrp_dBm > -140.0) {
                    outHeatmap.maxRSRP = std::max(outHeatmap.maxRSRP, hp.rsrp_dBm);
                    outHeatmap.minRSRP = std::min(outHeatmap.minRSRP, hp.rsrp_dBm);
                    sumRSRP += hp.rsrp_dBm;
                    validCount++;
                    if (hp.rsrp_dBm >= -100.0) {
                        // 覆盖良好
                    } else {
                        outHeatmap.weakCoverageCount++;
                    }
                }
            }
        }

        if (validCount > 0) {
            outHeatmap.avgRSRP = sumRSRP / validCount;
            outHeatmap.coverageRate = (double)(validCount - outHeatmap.weakCoverageCount) / validCount;
        }

        return ZF_ERR_OK;
    }

    // 多天线叠加（取最强信号）
    int generateMultiAntennaHeatmap(const Floor* floor,
                                    const std::vector<Point2D>& txPositions,
                                    HeatmapData& outHeatmap) {
        if (!floor || txPositions.empty()) return ZF_ERR_ARG;

        if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE) {
            return m_modeMgr->checkHeavyComputePermission();
        }

        // 先生成第一个天线的热力图
        int result = generateHeatmap(floor, txPositions[0], outHeatmap);
        if (result != ZF_ERR_OK) return result;

        // 叠加其他天线（取最强）
        for (size_t i = 1; i < txPositions.size(); i++) {
            HeatmapData temp;
            result = generateHeatmap(floor, txPositions[i], temp);
            if (result != ZF_ERR_OK) continue;

            for (size_t j = 0; j < outHeatmap.points.size() && j < temp.points.size(); j++) {
                if (temp.points[j].rsrp_dBm > outHeatmap.points[j].rsrp_dBm) {
                    outHeatmap.points[j] = temp.points[j];
                }
            }
        }

        // 重新统计
        recalcStats(outHeatmap);
        return ZF_ERR_OK;
    }

    // 热力图转文本矩阵
    std::string heatmapToText(const HeatmapData& heatmap, int cols = 40) const {
        if (heatmap.points.empty()) return "(空热力图)";

        std::string result;
        result += "热力图 [" + heatmap.floorId + "] " +
                  std::to_string(heatmap.gridWidth) + "x" + std::to_string(heatmap.gridHeight) + "\n";
        result += "RSRP范围: " + std::to_string(heatmap.minRSRP).substr(0,5) + " ~ " +
                  std::to_string(heatmap.maxRSRP).substr(0,5) + " dBm\n";
        result += "平均: " + std::to_string(heatmap.avgRSRP).substr(0,5) + " dBm, " +
                  "覆盖率: " + std::to_string(heatmap.coverageRate * 100).substr(0,4) + "%\n\n";

        // 简化显示
        int stepX = std::max(1, heatmap.gridWidth / cols);
        int stepY = std::max(1, heatmap.gridHeight / (cols / 2));

        for (int gy = 0; gy < heatmap.gridHeight; gy += stepY) {
            for (int gx = 0; gx < heatmap.gridWidth; gx += stepX) {
                int idx = gy * heatmap.gridWidth + gx;
                if (idx < (int)heatmap.points.size()) {
                    double rsrp = heatmap.points[idx].rsrp_dBm;
                    result += rsrpToChar(rsrp);
                }
            }
            result += "\n";
        }
        result += "\n图例: #强(>=-75) +中(>=-85) =弱(>=-95) .差(>=-105) 空无信号\n";
        return result;
    }

    void recalcStats(HeatmapData& heatmap) {
        heatmap.maxRSRP = -140;
        heatmap.minRSRP = 0;
        heatmap.avgRSRP = -140;
        heatmap.coverageRate = 0;
        heatmap.weakCoverageCount = 0;
        double sum = 0;
        int valid = 0;
        for (const auto& p : heatmap.points) {
            if (p.rsrp_dBm > -140) {
                heatmap.maxRSRP = std::max(heatmap.maxRSRP, p.rsrp_dBm);
                heatmap.minRSRP = std::min(heatmap.minRSRP, p.rsrp_dBm);
                sum += p.rsrp_dBm;
                valid++;
                if (p.rsrp_dBm < -100) heatmap.weakCoverageCount++;
            }
        }
        if (valid > 0) {
            heatmap.avgRSRP = sum / valid;
            heatmap.coverageRate = (double)(valid - heatmap.weakCoverageCount) / valid;
        }
    }

private:
    ModeManager* m_modeMgr{nullptr};
    SimulationConfig m_config;

    // 线段相交检测
    bool segmentsIntersect(const Point2D& p1, const Point2D& p2,
                           const Point2D& p3, const Point2D& p4) const {
        auto sub = [](const Point2D& a, const Point2D& b) {
            return Point2D{a.x - b.x, a.y - b.y};
        };
        double d1 = cross(sub(p4, p3), sub(p1, p3));
        double d2 = cross(sub(p4, p3), sub(p2, p3));
        double d3 = cross(sub(p2, p1), sub(p3, p1));
        double d4 = cross(sub(p2, p1), sub(p4, p1));

        if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
            ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
            return true;
        }
        return false;
    }

    double cross(const Point2D& a, const Point2D& b) const {
        return a.x * b.y - a.y * b.x;
    }

    Rect2D calcFloorBounds(const Floor* floor) const {
        Rect2D bounds;
        if (!floor || floor->walls.empty()) {
            bounds.origin = {0, 0};
            bounds.size = {10000, 10000};
            return bounds;
        }

        double minX = 1e18, minY = 1e18, maxX = -1e18, maxY = -1e18;
        for (const auto& wall : floor->walls) {
            for (const auto& p : wall.points) {
                minX = std::min(minX, p.x);
                minY = std::min(minY, p.y);
                maxX = std::max(maxX, p.x);
                maxY = std::max(maxY, p.y);
            }
        }
        bounds.origin = {minX, minY};
        bounds.size = {maxX - minX, maxY - minY};
        return bounds;
    }

    char rsrpToChar(double rsrp) const {
        if (rsrp >= -75) return '#';
        if (rsrp >= -85) return '+';
        if (rsrp >= -95) return '=';
        if (rsrp >= -105) return '.';
        if (rsrp >= -120) return '-';
        return ' ';
    }
};

} // namespace zf
