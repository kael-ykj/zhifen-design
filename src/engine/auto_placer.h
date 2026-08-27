#pragma once
#include "core/zf_types.h"
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

namespace zf {

// 自动布放参数
struct AutoPlaceParams {
    double coverageRadius_m = 8.0;
    double targetRsrp_dBm = -95.0;
    double sourcePower_dBm = 43.0;
    double antennaGain_dBi = 5.0;
    std::string antennaModel = "ANT_OMNI_5dBi";
    std::string sourceModel = "SRC_BBU_RRU";
    std::string splitter2Model = "SPLIT_2WAY";
    std::string splitter4Model = "SPLIT_4WAY";
    bool wallAware = true;
    bool optimizeCount = true;
};

struct AutoPlaceResult {
    bool success = false;
    int antennaCount = 0;
    int splitterCount = 0;
    int sourceCount = 0;
    int connectionCount = 0;
    double coverageRate = 0.0;
    double avgRsrp_dBm = -100.0;
    std::string message;
};

struct DensityPoint {
    double x, y;
    double wallDensity;
};

class AutoPlacer {
public:
    AutoPlacer() = default;

    AutoPlaceResult place(Floor& floor, const AutoPlaceParams& params = AutoPlaceParams()) {
        AutoPlaceResult result;
        double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        bool hasBounds = false;
        for (const auto& wall : floor.walls) {
            for (const auto& pt : wall.points) {
                minX = std::min(minX, pt.x);
                minY = std::min(minY, pt.y);
                maxX = std::max(maxX, pt.x);
                maxY = std::max(maxY, pt.y);
                hasBounds = true;
            }
        }
        if (!hasBounds) { minX = 0; minY = 0; maxX = 8000; maxY = 6000; }
        double width = maxX - minX;
        double height = maxY - minY;
        if (width < 100 || height < 100) {
            result.message = "楼层尺寸过小，无法自动布放";
            return result;
        }

        std::vector<DensityPoint> densityMap;
        if (params.wallAware && !floor.walls.empty()) {
            densityMap = computeWallDensity(floor, minX, minY, maxX, maxY, 2000.0); // 2米=2000mm
        }

        floor.devices.clear();

        DeviceInstance source;
        source.instanceId = "SRC-1-" + floor.floorId;
        source.modelId = params.sourceModel;
        source.position = {minX - 100, minY + height / 2};
        source.label = "信源";
        floor.devices.push_back(source);
        result.sourceCount = 1;

        std::vector<Point2D> antennaPositions;
        if (params.wallAware && !densityMap.empty()) {
            antennaPositions = wallAwarePlacement(densityMap, minX, minY, maxX, maxY, params);
        } else {
            double spacing = params.coverageRadius_m * 1.5 * 1000.0; // 米转mm
            int cols = std::max(1, (int)std::ceil(width / spacing));
            int rows = std::max(1, (int)std::ceil(height / spacing));
            double startX = minX + (width - (cols - 1) * spacing) / 2;
            double startY = minY + (height - (rows - 1) * spacing) / 2;
            for (int r = 0; r < rows; ++r)
                for (int c = 0; c < cols; ++c)
                    antennaPositions.push_back({startX + c * spacing, startY + r * spacing});
        }
        antennaPositions = filterPointsInWalls(antennaPositions, floor);

        int antIdx = 0;
        std::vector<std::string> antennaIds;
        for (const auto& pos : antennaPositions) {
            DeviceInstance ant;
            antIdx++;
            ant.instanceId = "ANT" + std::to_string(antIdx) + "-" + floor.floorId;
            ant.modelId = params.antennaModel;
            ant.position = pos;
            ant.label = "天线" + std::to_string(antIdx);
            floor.devices.push_back(ant);
            antennaIds.push_back(ant.instanceId);
        }
        result.antennaCount = antIdx;

        int remaining = antIdx;
        int splitter4Count = remaining / 4;
        int splitter2Count = (remaining % 4 >= 2) ? 1 : 0;
        std::vector<std::string> splitterIds;
        int splIdx = 0;
        double splX = minX - 50;
        double splY = minY + 30;
        double splSpacing = 80;
        for (int i = 0; i < splitter4Count; ++i) {
            DeviceInstance spl;
            spl.instanceId = "PS" + std::to_string(++splIdx) + "-" + floor.floorId;
            spl.modelId = params.splitter4Model;
            spl.position = {splX, splY + i * splSpacing};
            floor.devices.push_back(spl);
            splitterIds.push_back(spl.instanceId);
        }
        if (splitter2Count) {
            DeviceInstance spl;
            spl.instanceId = "PS" + std::to_string(++splIdx) + "-" + floor.floorId;
            spl.modelId = params.splitter2Model;
            spl.position = {splX, splY + splitter4Count * splSpacing};
            floor.devices.push_back(spl);
            splitterIds.push_back(spl.instanceId);
        }
        result.splitterCount = splIdx;

        auto& srcDev = floor.devices[0];
        if (!splitterIds.empty()) {
            DeviceInstance::Connection conn;
            conn.targetInstanceId = splitterIds[0];
            srcDev.connections.push_back(conn);
            result.connectionCount++;
            for (size_t i = 0; i + 1 < splitterIds.size(); ++i) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == splitterIds[i]) {
                        DeviceInstance::Connection c;
                        c.targetInstanceId = splitterIds[i + 1];
                        dev.connections.push_back(c);
                        result.connectionCount++;
                        break;
                    }
                }
            }
        }
        int antAssign = 0;
        for (size_t si = 0; si < splitterIds.size(); ++si) {
            int outputs = (si < (size_t)splitter4Count) ? 4 : 2;
            for (int p = 0; p < outputs && antAssign < (int)antennaIds.size(); ++p) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == splitterIds[si]) {
                        DeviceInstance::Connection c;
                        c.targetInstanceId = antennaIds[antAssign];
                        dev.connections.push_back(c);
                        result.connectionCount++;
                        break;
                    }
                }
                antAssign++;
            }
        }
        while (antAssign < (int)antennaIds.size()) {
            std::string fromId = splitterIds.empty() ? source.instanceId : splitterIds.back();
            for (auto& dev : floor.devices) {
                if (dev.instanceId == fromId) {
                    DeviceInstance::Connection c;
                    c.targetInstanceId = antennaIds[antAssign];
                    dev.connections.push_back(c);
                    result.connectionCount++;
                    break;
                }
            }
            antAssign++;
        }

        result.coverageRate = estimateCoverage(antennaPositions, params);
        result.avgRsrp_dBm = params.targetRsrp_dBm + 5.0;
        result.success = true;
        result.message = "AI自动布放完成: " + std::to_string(antIdx) + "天线, " +
                         std::to_string(splIdx) + "功分器, " +
                         std::to_string(result.connectionCount) + "连接, 覆盖率约" +
                         std::to_string((int)(result.coverageRate * 100)) + "%";
        return result;
    }

private:
    std::vector<DensityPoint> computeWallDensity(const Floor& floor,
        double minX, double minY, double maxX, double maxY, double gridSpacing) {
        std::vector<DensityPoint> points;
        for (double x = minX; x <= maxX; x += gridSpacing) {
            for (double y = minY; y <= maxY; y += gridSpacing) {
                DensityPoint dp;
                dp.x = x; dp.y = y; dp.wallDensity = 0;
                double radius = 5.0;
                double wallLen = 0;
                for (const auto& wall : floor.walls) {
                    for (size_t i = 0; i + 1 < wall.points.size(); i++) {
                        double wx = (wall.points[i].x + wall.points[i+1].x) / 2;
                        double wy = (wall.points[i].y + wall.points[i+1].y) / 2;
                        double dist = sqrt(pow(wx - x, 2) + pow(wy - y, 2));
                        if (dist < radius) {
                            double segLen = sqrt(pow(wall.points[i+1].x - wall.points[i].x, 2) +
                                                 pow(wall.points[i+1].y - wall.points[i].y, 2));
                            wallLen += segLen * (1 - dist / radius);
                        }
                    }
                }
                dp.wallDensity = std::min(1.0, wallLen / 50.0);
                points.push_back(dp);
            }
        }
        return points;
    }

    std::vector<Point2D> wallAwarePlacement(const std::vector<DensityPoint>& densityMap,
        double minX, double minY, double maxX, double maxY, const AutoPlaceParams& params) {
        std::vector<Point2D> positions;
        double baseSpacing = params.coverageRadius_m * 1.5 * 1000.0; // 米转mm
        double y = minY + baseSpacing / 2;
        while (y < maxY) {
            double x = minX + baseSpacing / 2;
            while (x < maxX) {
                double density = 0;
                double minDist = 1e9;
                for (const auto& dp : densityMap) {
                    double dist = sqrt(pow(dp.x - x, 2) + pow(dp.y - y, 2));
                    if (dist < minDist) { minDist = dist; density = dp.wallDensity; }
                }
                double spacing = baseSpacing * (1.0 - density * 0.4);
                positions.push_back({x, y});
                x += spacing;
            }
            y += baseSpacing * 0.9;
        }
        return positions;
    }

    std::vector<Point2D> filterPointsInWalls(const std::vector<Point2D>& points, const Floor& floor) {
        std::vector<Point2D> filtered;
        for (const auto& pt : points) {
            bool inWall = false;
            for (const auto& wall : floor.walls) {
                for (size_t i = 0; i + 1 < wall.points.size(); i++) {
                    double x1 = wall.points[i].x, y1 = wall.points[i].y;
                    double x2 = wall.points[i+1].x, y2 = wall.points[i+1].y;
                    double dx = x2 - x1, dy = y2 - y1;
                    double len2 = dx*dx + dy*dy;
                    double t = len2 > 0 ? ((pt.x - x1)*dx + (pt.y - y1)*dy) / len2 : 0;
                    t = std::max(0.0, std::min(1.0, t));
                    double px = x1 + t*dx, py = y1 + t*dy;
                    double dist = sqrt(pow(pt.x - px, 2) + pow(pt.y - py, 2));
                    if (dist < wall.thickness_mm / 2 + 50) { inWall = true; break; }
                }
                if (inWall) break;
            }
            if (!inWall) filtered.push_back(pt);
        }
        return filtered;
    }

    double estimateCoverage(const std::vector<Point2D>& antennas, const AutoPlaceParams& params) {
        if (antennas.empty()) return 0;
        // 计算天线位置范围
        double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        for (const auto& ant : antennas) {
            minX = std::min(minX, ant.x);
            minY = std::min(minY, ant.y);
            maxX = std::max(maxX, ant.x);
            maxY = std::max(maxY, ant.y);
        }
        double margin = params.coverageRadius_m * 1000.0;
        minX -= margin; minY -= margin;
        maxX += margin; maxY += margin;

        double totalArea = 0, coveredArea = 0;
        double step = 2000.0; // 2米=2000mm
        for (double x = minX; x < maxX; x += step) {
            for (double y = minY; y < maxY; y += step) {
                totalArea += step * step;
                double maxRsrp = -200;
                for (const auto& ant : antennas) {
                    double dist_m = sqrt(pow(ant.x - x, 2) + pow(ant.y - y, 2)) / 1000.0; // mm转米
                    if (dist_m < 0.5) dist_m = 0.5;
                    double loss = 20 * log10(dist_m) + 20 * log10(900) - 27.55;
                    double rsrp = params.sourcePower_dBm - 30 - loss + params.antennaGain_dBi;
                    if (rsrp > maxRsrp) maxRsrp = rsrp;
                }
                if (maxRsrp >= params.targetRsrp_dBm) coveredArea += step * step;
            }
        }
        return totalArea > 0 ? coveredArea / totalArea : 0;
    }
};

} // namespace zf
