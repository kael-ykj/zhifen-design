#include "route_planner.h"
#include "../entities/caditem.h"
#include "../devices/deviceitem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QLineF>
#include <QSet>
#include <QHash>
#include <QtMath>
#include <QDebug>

namespace Zhifen {

RoutePlanner::RoutePlanner() {}
RoutePlanner::~RoutePlanner() {}

void RoutePlanner::setScene(QGraphicsScene *scene) {
    m_scene = scene;
    detectWalls();
    detectObstacles();
}

void RoutePlanner::detectWalls() {
    m_walls.clear();
    if (!m_scene) return;

    // 简化：从场景中的直线/多段线检测墙体
    // 实际项目中应从DXF导入的图层中识别墙体图层
    for (auto *item : m_scene->items()) {
        auto *cad = dynamic_cast<CadItem*>(item);
        if (!cad) continue;
        // 墙体识别：长度较长的直线（简化逻辑）
        // 实际应基于图层名称或图元类型
    }
}

void RoutePlanner::detectObstacles() {
    m_obstacles.clear();
    if (!m_scene) return;
    // 简化：实际应从场景中识别门窗、电梯井等障碍物
}

void RoutePlanner::addObstacle(const QRectF &rect, const QString &type) {
    Obstacle obs;
    obs.rect = rect;
    obs.type = type;
    m_obstacles.append(obs);
}

void RoutePlanner::clearObstacles() {
    m_obstacles.clear();
}

bool RoutePlanner::isWalkable(const QPointF &point) const {
    for (const auto &obs : m_obstacles) {
        if (obs.rect.contains(point)) return false;
    }
    return true;
}

bool RoutePlanner::crossesObstacle(const QPointF &p1, const QPointF &p2) const {
    QLineF line(p1, p2);
    for (const auto &obs : m_obstacles) {
        // 简化：检查线段是否与障碍物矩形相交
        QRectF r = obs.rect;
        QLineF top(r.topLeft(), r.topRight());
        QLineF bottom(r.bottomLeft(), r.bottomRight());
        QLineF left(r.topLeft(), r.bottomLeft());
        QLineF right(r.topRight(), r.bottomRight());
        QPointF intersect;
        if (line.intersects(top, &intersect) == QLineF::BoundedIntersection ||
            line.intersects(bottom, &intersect) == QLineF::BoundedIntersection ||
            line.intersects(left, &intersect) == QLineF::BoundedIntersection ||
            line.intersects(right, &intersect) == QLineF::BoundedIntersection) {
            return true;
        }
    }
    return false;
}

qreal RoutePlanner::manhattanHeuristic(const QPointF &a, const QPointF &b) const {
    return qAbs(a.x() - b.x()) + qAbs(a.y() - b.y());
}

qreal RoutePlanner::euclideanDistance(const QPointF &a, const QPointF &b) const {
    return qSqrt(qPow(a.x() - b.x(), 2) + qPow(a.y() - b.y(), 2));
}

QList<QPointF> RoutePlanner::aStarSearch(const QPointF &start, const QPointF &end) {
    // 简化版A*：使用网格搜索
    // 实际项目中应使用更高效的实现
    QList<QPointF> path;

    if (!isWalkable(start) || !isWalkable(end)) {
        return path;
    }

    // 简化：直接生成横平竖直的路径（曼哈顿路由）
    if (m_config.preference == Route_Manhattan) {
        path.append(start);
        // 先水平后垂直
        QPointF mid(end.x(), start.y());
        if (isWalkable(mid) && !crossesObstacle(start, mid) && !crossesObstacle(mid, end)) {
            path.append(mid);
        } else {
            // 先垂直后水平
            mid = QPointF(start.x(), end.y());
            path.append(mid);
        }
        path.append(end);
    } else {
        // 最短路径（直线）
        path.append(start);
        path.append(end);
    }

    return path;
}

QList<QPointF> RoutePlanner::optimizeAlongWall(const QList<QPointF> &path) {
    // 简化：沿墙优化
    // 实际项目中应检测墙体并调整路由沿墙走线
    return path;
}

QList<RouteSegment> RoutePlanner::buildSegments(const QList<QPointF> &waypoints) {
    QList<RouteSegment> segments;
    for (int i = 0; i < waypoints.size() - 1; i++) {
        RouteSegment seg;
        seg.start = waypoints[i];
        seg.end = waypoints[i + 1];
        seg.length = euclideanDistance(seg.start, seg.end);
        seg.feederType = m_config.defaultFeeder;
        segments.append(seg);
    }
    return segments;
}

int RoutePlanner::countBends(const QList<QPointF> &waypoints) const {
    int bends = 0;
    for (int i = 1; i < waypoints.size() - 1; i++) {
        QPointF prev = waypoints[i - 1];
        QPointF curr = waypoints[i];
        QPointF next = waypoints[i + 1];
        // 检查方向是否改变
        qreal dx1 = curr.x() - prev.x();
        qreal dy1 = curr.y() - prev.y();
        qreal dx2 = next.x() - curr.x();
        qreal dy2 = next.y() - curr.y();
        if ((dx1 != 0 && dy2 != 0) || (dy1 != 0 && dx2 != 0)) {
            bends++;
        }
    }
    return bends;
}

RouteResult RoutePlanner::planRoute(const QPointF &start, const QPointF &end) {
    RouteResult result;

    // A*寻路
    QList<QPointF> path = aStarSearch(start, end);
    if (path.isEmpty()) {
        result.success = false;
        result.warnings.append("无法找到可行路径");
        return result;
    }

    // 沿墙优化
    path = optimizeAlongWall(path);

    // 构建路由段
    result.waypoints = path;
    result.segments = buildSegments(path);

    // 统计
    result.totalLength = 0;
    for (const auto &seg : result.segments) {
        result.totalLength += seg.length;
    }
    result.bendCount = countBends(path);

    result.success = true;
    return result;
}

QList<RouteResult> RoutePlanner::planBatchRoutes(const QPointF &start, const QList<QPointF> &ends) {
    QList<RouteResult> results;
    for (const auto &end : ends) {
        results.append(planRoute(start, end));
    }
    return results;
}

} // namespace Zhifen
