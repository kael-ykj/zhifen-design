#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {

// 校准点对
struct CalibrationPoint {
    Point2D imageCoord;   // 底图像素坐标
    Point2D worldCoord;   // 实际工程坐标
};

// 仿射变换矩阵（2x3）
struct AffineTransform {
    double a{1}, b{0}, c{0};  // x' = a*x + b*y + c
    double d{0}, e{1}, f{0};  // y' = d*x + e*y + f

    Point2D apply(const Point2D& p) const {
        return {a * p.x + b * p.y + c, d * p.x + e * p.y + f};
    }

    double scaleX() const { return std::sqrt(a*a + d*d); }
    double scaleY() const { return std::sqrt(b*b + e*e); }
    double rotation() const { return std::atan2(d, a); }
};

// 墙体建模引擎
class BuildingModelEngine {
public:
    BuildingModelEngine() = default;

    // 创建墙体
    Wall createWall(const std::string& wallId, const std::vector<Point2D>& points,
                    WallMaterial material = WallMaterial::CONCRETE,
                    double thickness_mm = 200.0) {
        Wall wall;
        wall.wallId = wallId;
        wall.material = material;
        wall.thickness_mm = thickness_mm;
        wall.points = points;
        wall.attenuation_dB = getWallAttenuation(material);
        return wall;
    }

    // 获取墙体材料衰减值（dB）
    double getWallAttenuation(WallMaterial material) const {
        switch (material) {
            case WallMaterial::CONCRETE: return 12.0;
            case WallMaterial::BRICK: return 8.0;
            case WallMaterial::GLASS: return 3.0;
            case WallMaterial::METAL: return 20.0;
            case WallMaterial::DRYWALL: return 2.0;
            default: return 10.0;
        }
    }

    // 计算墙体总长度（米，假设坐标单位为毫米，除以1000）
    double calcWallLength(const Wall& wall, double scale = 1000.0) const {
        double total = 0;
        for (size_t i = 0; i + 1 < wall.points.size(); i++) {
            double dx = wall.points[i+1].x - wall.points[i].x;
            double dy = wall.points[i+1].y - wall.points[i].y;
            total += std::sqrt(dx*dx + dy*dy);
        }
        return total / scale;
    }

    // 从墙体自动识别房间（封闭区域）
    int autoDetectRooms(Floor* floor) {
        if (!floor) return ZF_ERR_ARG;
        floor->rooms.clear();

        // 收集所有线段
        struct Edge {
            Point2D p0, p1;
            std::string wallId;
        };
        std::vector<Edge> edges;
        for (const auto& wall : floor->walls) {
            for (size_t i = 0; i + 1 < wall.points.size(); i++) {
                edges.push_back({wall.points[i], wall.points[i+1], wall.wallId});
            }
        }

        if (edges.empty()) return ZF_ERR_OK;

        // 构建邻接表（按端点）
        auto pointKey = [](const Point2D& p) {
            return std::to_string((int)(p.x * 100)) + "," + std::to_string((int)(p.y * 100));
        };

        std::map<std::string, std::vector<int>> adj;
        for (size_t i = 0; i < edges.size(); i++) {
            adj[pointKey(edges[i].p0)].push_back(i);
            adj[pointKey(edges[i].p1)].push_back(i);
        }

        // 找环（简化版：用DFS找封闭多边形）
        std::set<int> visitedEdges;
        int roomCount = 0;

        for (size_t start = 0; start < edges.size(); start++) {
            if (visitedEdges.count(start)) continue;

            std::vector<Point2D> polygon;
            std::vector<int> pathEdges;
            Point2D current = edges[start].p0;
            Point2D startPoint = current;
            int currentEdge = start;

            int safety = 0;
            while (safety++ < 100) {
                polygon.push_back(current);
                pathEdges.push_back(currentEdge);
                visitedEdges.insert(currentEdge);

                // 找下一条边
                Point2D nextPoint = (pointKey(edges[currentEdge].p0) == pointKey(current))
                    ? edges[currentEdge].p1 : edges[currentEdge].p0;

                if (pointKey(nextPoint) == pointKey(startPoint) && pathEdges.size() > 2) {
                    // 找到封闭环
                    Room room;
                    room.roomId = "ROOM_AUTO_" + std::to_string(roomCount++);
                    room.name = "房间" + std::to_string(roomCount);
                    room.polygon = polygon;
                    room.area_m2 = calcPolygonArea(polygon) / 1000000.0; // 假设毫米转平方米
                    floor->rooms.push_back(room);
                    break;
                }

                // 从邻接边中找未访问的
                bool found = false;
                for (int eIdx : adj[pointKey(nextPoint)]) {
                    if (!visitedEdges.count(eIdx) && eIdx != currentEdge) {
                        currentEdge = eIdx;
                        current = nextPoint;
                        found = true;
                        break;
                    }
                }
                if (!found) break;
            }
        }

        return ZF_ERR_OK;
    }

    // 计算多边形面积（鞋带公式）
    double calcPolygonArea(const std::vector<Point2D>& polygon) const {
        if (polygon.size() < 3) return 0;
        double area = 0;
        for (size_t i = 0; i < polygon.size(); i++) {
            size_t j = (i + 1) % polygon.size();
            area += polygon[i].x * polygon[j].y;
            area -= polygon[j].x * polygon[i].y;
        }
        return std::abs(area) / 2.0;
    }

    // 从校准点计算仿射变换（至少3对点）
    int computeAffineTransform(const std::vector<CalibrationPoint>& points,
                               AffineTransform& outTransform) {
        if (points.size() < 3) return ZF_ERR_ARG;

        // 最小二乘法求解仿射变换
        // x' = a*x + b*y + c
        // y' = d*x + e*y + f
        // 用前3个点精确求解
        const auto& p0 = points[0];
        const auto& p1 = points[1];
        const auto& p2 = points[2];

        double x0 = p0.imageCoord.x, y0 = p0.imageCoord.y;
        double x1 = p1.imageCoord.x, y1 = p1.imageCoord.y;
        double x2 = p2.imageCoord.x, y2 = p2.imageCoord.y;

        double X0 = p0.worldCoord.x, Y0 = p0.worldCoord.y;
        double X1 = p1.worldCoord.x, Y1 = p1.worldCoord.y;
        double X2 = p2.worldCoord.x, Y2 = p2.worldCoord.y;

        double det = x0*(y1-y2) - y0*(x1-x2) + (x1*y2 - x2*y1);
        if (std::abs(det) < 1e-10) return ZF_ERR_ARG;

        outTransform.a = ((X0-X2)*(y1-y2) - (y0-y2)*(X1-X2)) / det;
        outTransform.b = ((x0-x2)*(X1-X2) - (X0-X2)*(x1-x2)) / det;
        outTransform.c = (X0*(x1*y2 - x2*y1) - x0*(X1*y2 - X2*y1) + y0*(X1*x2 - X2*x1)) / det;

        outTransform.d = ((Y0-Y2)*(y1-y2) - (y0-y2)*(Y1-Y2)) / det;
        outTransform.e = ((x0-x2)*(Y1-Y2) - (Y0-Y2)*(x1-x2)) / det;
        outTransform.f = (Y0*(x1*y2 - x2*y1) - x0*(Y1*y2 - Y2*y1) + y0*(Y1*x2 - Y2*x1)) / det;

        return ZF_ERR_OK;
    }

    // 应用底图校准到楼层
    int applyBackgroundCalibration(Floor* floor, const AffineTransform& transform) {
        if (!floor) return ZF_ERR_ARG;
        floor->origin = {transform.c, transform.f};
        floor->drawingScale = transform.scaleX() * 1000.0; // 假设比例
        return ZF_ERR_OK;
    }

    // 点是否在房间内（射线法）
    bool pointInRoom(const Point2D& point, const Room& room) const {
        if (room.polygon.size() < 3) return false;
        bool inside = false;
        for (size_t i = 0, j = room.polygon.size() - 1; i < room.polygon.size(); j = i++) {
            if (((room.polygon[i].y > point.y) != (room.polygon[j].y > point.y)) &&
                (point.x < (room.polygon[j].x - room.polygon[i].x) *
                 (point.y - room.polygon[i].y) / (room.polygon[j].y - room.polygon[i].y) +
                 room.polygon[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }

    // 统计楼层信息
    struct FloorStats {
        int wallCount{0};
        double totalWallLength_m{0.0};
        int roomCount{0};
        double totalRoomArea_m2{0.0};
        int deviceCount{0};
    };

    FloorStats calcFloorStats(const Floor* floor) const {
        FloorStats stats;
        if (!floor) return stats;
        stats.wallCount = floor->walls.size();
        for (const auto& w : floor->walls) stats.totalWallLength_m += calcWallLength(w);
        stats.roomCount = floor->rooms.size();
        for (const auto& r : floor->rooms) stats.totalRoomArea_m2 += r.area_m2;
        stats.deviceCount = floor->devices.size();
        return stats;
    }
};

} // namespace zf
