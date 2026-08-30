#ifndef GRIPMANAGER_H
#define GRIPMANAGER_H

#include <QObject>
#include <QPointF>
#include <QList>
#include <QMap>
#include <QPainter>
#include <QGraphicsItem>

class CadScene;
class CadView;
class CadItem;
class LineItem;
class CircleItem;
class ArcItem;
class PolylineItem;
class RectangleItem;

namespace Zhifen {

// 夹点类型
enum GripType {
    Grip_Endpoint = 0,      // 端点
    Grip_Midpoint = 1,      // 中点
    Grip_Center = 2,        // 圆心
    Grip_Quadrant = 3,      // 象限点
    Grip_Vertex = 4,        // 顶点
    Grip_Insertion = 5,     // 插入点
    Grip_Radius = 6,        // 半径点
    Grip_Angle = 7,         // 角度点
    Grip_Custom = 8         // 自定义
};

// 夹点数据
struct GripPoint {
    QPointF position;
    GripType type;
    int index = -1;         // 图元中的索引（如多段线顶点索引）
    CadItem *item = nullptr;
    bool isHot = false;     // 是否为热夹点（被选中）
};

// 夹点管理器
class GripManager : public QObject
{
    Q_OBJECT
public:
    explicit GripManager(CadScene *scene, CadView *view, QObject *parent = nullptr);

    // 启用/禁用
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // 更新夹点（基于当前选中的图元）
    void updateGrips();

    // 清除夹点
    void clearGrips();

    // 获取夹点
    QList<GripPoint*> grips() const { return m_grips; }
    bool hasGrips() const { return !m_grips.isEmpty(); }

    // 计算夹点（根据图元类型）
    QList<GripPoint*> computeGrips(CadItem *item);

    // 查找鼠标位置下的夹点
    GripPoint* gripAt(const QPointF &worldPos);

    // 热夹点（当前正在拖拽的夹点）
    void setHotGrip(GripPoint *grip) { m_hotGrip = grip; if (grip) grip->isHot = true; }
    GripPoint* hotGrip() const { return m_hotGrip; }
    void clearHotGrip() { if (m_hotGrip) m_hotGrip->isHot = false; m_hotGrip = nullptr; }

    // 拖拽夹点
    void dragGrip(GripPoint *grip, const QPointF &newPos);

    // 绘制夹点
    void drawGrips(QPainter *painter);

    // 夹点大小（屏幕像素）
    void setGripSize(int size) { m_gripSize = size; }
    int gripSize() const { return m_gripSize; }

signals:
    void gripChanged();
    void gripDragStarted(GripPoint *grip);
    void gripDragEnded(GripPoint *grip);

private:
    CadScene *m_scene;
    CadView *m_view;
    bool m_enabled = true;
    QList<GripPoint*> m_grips;
    GripPoint *m_hotGrip = nullptr;
    int m_gripSize = 8;
    qreal m_tolerance = 8.0; // 屏幕像素

    // 具体图元的夹点计算
    QList<GripPoint*> computeLineGrips(LineItem *line);
    QList<GripPoint*> computeCircleGrips(CircleItem *circle);
    QList<GripPoint*> computeArcGrips(ArcItem *arc);
    QList<GripPoint*> computePolylineGrips(PolylineItem *poly);
    QList<GripPoint*> computeRectangleGrips(RectangleItem *rect);

    // 具体图元的夹点拖拽
    void dragLineGrip(LineItem *line, GripPoint *grip, const QPointF &newPos);
    void dragCircleGrip(CircleItem *circle, GripPoint *grip, const QPointF &newPos);
    void dragArcGrip(ArcItem *arc, GripPoint *grip, const QPointF &newPos);
    void dragPolylineGrip(PolylineItem *poly, GripPoint *grip, const QPointF &newPos);
    void dragRectangleGrip(RectangleItem *rect, GripPoint *grip, const QPointF &newPos);
};

} // namespace Zhifen

#endif // GRIPMANAGER_H
