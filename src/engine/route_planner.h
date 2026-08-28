#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H

#include <QString>
#include <QList>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QGraphicsScene>

namespace Zhifen {

// 路由节点
struct RouteNode {
    QPointF pos;
    qreal g = 0;  // 从起点到当前节点的代价
    qreal h = 0;  // 启发式估计到终点的代价
    qreal f = 0;  // f = g + h
    RouteNode *parent = nullptr;
    bool closed = false;
    bool opened = false;
};

// 路由段
struct RouteSegment {
    QPointF start;
    QPointF end;
    qreal length = 0;
    QString feederType = "1/2馈线";
};

// 路由结果
struct RouteResult {
    bool success = false;
    QList<QPointF> waypoints;  // 路由经过点（含起点终点）
    QList<RouteSegment> segments;
    qreal totalLength = 0;
    int bendCount = 0;         // 弯头数量
    QStringList warnings;
};

// 布线偏好
enum RoutePreference {
    Route_Manhattan = 0,   // 横平竖直
    Route_Shortest = 1,    // 最短路径
    Route_AlongWall = 2    // 沿墙走线
};

// 布线参数
struct RouteConfig {
    qreal gridSize = 0.1;          // 网格大小（米）
    qreal wallOffset = 0.05;       // 离墙距离（米）
    qreal bendRadius = 0.1;        // 弯头半径（米）
    RoutePreference preference = Route_Manhattan;
    bool avoidDoors = true;        // 避让门窗
    bool avoidElevator = true;     // 避让电梯井
    QString defaultFeeder = "1/2馈线";
};

// 障碍物
struct Obstacle {
    QRectF rect;
    QString type;  // door/window/elevator/other
};

// 智能路由规划器
class RoutePlanner
{
public:
    RoutePlanner();
    ~RoutePlanner();

    // 设置场景（自动检测墙体和障碍物）
    void setScene(QGraphicsScene *scene);

    // 设置布线参数
    void setConfig(const RouteConfig &config) { m_config = config; }
    RouteConfig config() const { return m_config; }

    // 规划单条路由
    RouteResult planRoute(const QPointF &start, const QPointF &end);

    // 批量规划（多个终点共享主干）
    QList<RouteResult> planBatchRoutes(const QPointF &start, const QList<QPointF> &ends);

    // 手动添加障碍物
    void addObstacle(const QRectF &rect, const QString &type);
    void clearObstacles();

    // 获取检测到的墙体
    QList<QLineF> detectedWalls() const { return m_walls; }
    QList<Obstacle> detectedObstacles() const { return m_obstacles; }

private:
    RouteConfig m_config;
    QGraphicsScene *m_scene = nullptr;
    QList<QLineF> m_walls;
    QList<Obstacle> m_obstacles;

    // 从场景检测墙体
    void detectWalls();

    // 从场景检测障碍物
    void detectObstacles();

    // A*寻路
    QList<QPointF> aStarSearch(const QPointF &start, const QPointF &end);

    // 检查点是否可通行
    bool isWalkable(const QPointF &point) const;

    // 检查线段是否穿过障碍物
    bool crossesObstacle(const QPointF &p1, const QPointF &p2) const;

    // 沿墙优化路由
    QList<QPointF> optimizeAlongWall(const QList<QPointF> &path);

    // 计算路由段
    QList<RouteSegment> buildSegments(const QList<QPointF> &waypoints);

    // 统计弯头
    int countBends(const QList<QPointF> &waypoints) const;

    // 曼哈顿距离启发函数
    qreal manhattanHeuristic(const QPointF &a, const QPointF &b) const;

    // 欧几里得距离
    qreal euclideanDistance(const QPointF &a, const QPointF &b) const;
};

} // namespace Zhifen

#endif // ROUTE_PLANNER_H
