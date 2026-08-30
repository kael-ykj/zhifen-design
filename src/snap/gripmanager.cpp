#include "gripmanager.h"
#include "cadscene.h"
#include "cadview.h"
#include "caditem.h"
#include "lineitem.h"
#include "circleitem.h"
#include "arcitem.h"
#include "polylineitem.h"
#include "rectangleitem.h"
#include <QGraphicsScene>
#include <QtMath>

namespace Zhifen {

GripManager::GripManager(CadScene *scene, CadView *view, QObject *parent)
    : QObject(parent), m_scene(scene), m_view(view)
{
}

void GripManager::updateGrips()
{
    clearGrips();
    if (!m_enabled || !m_scene) return;

    auto selectedItems = m_scene->selectedItems();
    for (auto item : selectedItems) {
        CadItem *cadItem = dynamic_cast<CadItem*>(item);
        if (cadItem) {
            auto grips = computeGrips(cadItem);
            m_grips.append(grips);
        }
    }
    emit gripChanged();
}

void GripManager::clearGrips()
{
    clearHotGrip();
    qDeleteAll(m_grips);
    m_grips.clear();
    emit gripChanged();
}

QList<GripPoint*> GripManager::computeGrips(CadItem *item)
{
    QList<GripPoint*> grips;
    if (!item) return grips;

    if (auto line = dynamic_cast<LineItem*>(item)) {
        grips = computeLineGrips(line);
    } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
        grips = computeCircleGrips(circle);
    } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
        grips = computeArcGrips(arc);
    } else if (auto poly = dynamic_cast<PolylineItem*>(item)) {
        grips = computePolylineGrips(poly);
    } else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
        grips = computeRectangleGrips(rect);
    } else {
        // 默认：中心点作为插入点夹点
        GripPoint *g = new GripPoint{item->center(), Grip_Insertion, -1, item, false};
        grips.append(g);
    }
    return grips;
}

QList<GripPoint*> GripManager::computeLineGrips(LineItem *line)
{
    QList<GripPoint*> grips;
    // 两个端点
    grips.append(new GripPoint{line->startPoint(), Grip_Endpoint, 0, line, false});
    grips.append(new GripPoint{line->endPoint(), Grip_Endpoint, 1, line, false});
    // 中点
    QPointF mid = (line->startPoint() + line->endPoint()) / 2;
    grips.append(new GripPoint{mid, Grip_Midpoint, -1, line, false});
    return grips;
}

QList<GripPoint*> GripManager::computeCircleGrips(CircleItem *circle)
{
    QList<GripPoint*> grips;
    QPointF c = circle->centerPoint();
    qreal r = circle->radius();
    // 圆心
    grips.append(new GripPoint{c, Grip_Center, -1, circle, false});
    // 四个象限点
    grips.append(new GripPoint{QPointF(c.x() + r, c.y()), Grip_Quadrant, 0, circle, false});
    grips.append(new GripPoint{QPointF(c.x(), c.y() + r), Grip_Quadrant, 1, circle, false});
    grips.append(new GripPoint{QPointF(c.x() - r, c.y()), Grip_Quadrant, 2, circle, false});
    grips.append(new GripPoint{QPointF(c.x(), c.y() - r), Grip_Quadrant, 3, circle, false});
    return grips;
}

QList<GripPoint*> GripManager::computeArcGrips(ArcItem *arc)
{
    QList<GripPoint*> grips;
    // 起点、终点
    grips.append(new GripPoint{arc->startPoint(), Grip_Endpoint, 0, arc, false});
    grips.append(new GripPoint{arc->endPoint(), Grip_Endpoint, 1, arc, false});
    // 圆心
    grips.append(new GripPoint{arc->centerPoint(), Grip_Center, -1, arc, false});
    // 中点（弧上）
    qreal midAngle = (arc->startAngle() + arc->endAngle()) / 2;
    QPointF mid(arc->centerPoint().x() + arc->radius() * qCos(midAngle),
                arc->centerPoint().y() + arc->radius() * qSin(midAngle));
    grips.append(new GripPoint{mid, Grip_Midpoint, -1, arc, false});
    return grips;
}

QList<GripPoint*> GripManager::computePolylineGrips(PolylineItem *poly)
{
    QList<GripPoint*> grips;
    auto points = poly->points();
    for (int i = 0; i < points.size(); i++) {
        grips.append(new GripPoint{points[i], Grip_Vertex, i, poly, false});
    }
    return grips;
}

QList<GripPoint*> GripManager::computeRectangleGrips(RectangleItem *rect)
{
    QList<GripPoint*> grips;
    QRectF r = rect->rect();
    // 四个角点
    grips.append(new GripPoint{r.topLeft(), Grip_Vertex, 0, rect, false});
    grips.append(new GripPoint{r.topRight(), Grip_Vertex, 1, rect, false});
    grips.append(new GripPoint{r.bottomRight(), Grip_Vertex, 2, rect, false});
    grips.append(new GripPoint{r.bottomLeft(), Grip_Vertex, 3, rect, false});
    // 四条边中点
    grips.append(new GripPoint{(r.topLeft() + r.topRight()) / 2, Grip_Midpoint, 4, rect, false});
    grips.append(new GripPoint{(r.topRight() + r.bottomRight()) / 2, Grip_Midpoint, 5, rect, false});
    grips.append(new GripPoint{(r.bottomRight() + r.bottomLeft()) / 2, Grip_Midpoint, 6, rect, false});
    grips.append(new GripPoint{(r.bottomLeft() + r.topLeft()) / 2, Grip_Midpoint, 7, rect, false});
    return grips;
}

GripPoint* GripManager::gripAt(const QPointF &worldPos)
{
    if (!m_view) return nullptr;
    qreal worldTolerance = m_tolerance / m_view->transform().m11();
    for (auto grip : m_grips) {
        qreal d = QLineF(worldPos, grip->position).length();
        if (d < worldTolerance) {
            return grip;
        }
    }
    return nullptr;
}

void GripManager::dragGrip(GripPoint *grip, const QPointF &newPos)
{
    if (!grip || !grip->item) return;

    if (auto line = dynamic_cast<LineItem*>(grip->item)) {
        dragLineGrip(line, grip, newPos);
    } else if (auto circle = dynamic_cast<CircleItem*>(grip->item)) {
        dragCircleGrip(circle, grip, newPos);
    } else if (auto arc = dynamic_cast<ArcItem*>(grip->item)) {
        dragArcGrip(arc, grip, newPos);
    } else if (auto poly = dynamic_cast<PolylineItem*>(grip->item)) {
        dragPolylineGrip(poly, grip, newPos);
    } else if (auto rect = dynamic_cast<RectangleItem*>(grip->item)) {
        dragRectangleGrip(rect, grip, newPos);
    }

    // 更新夹点位置
    grip->position = newPos;
    updateGrips();
}

void GripManager::dragLineGrip(LineItem *line, GripPoint *grip, const QPointF &newPos)
{
    if (grip->type == Grip_Endpoint) {
        if (grip->index == 0) {
            line->setLine(newPos, line->endPoint());
        } else if (grip->index == 1) {
            line->setLine(line->startPoint(), newPos);
        }
    } else if (grip->type == Grip_Midpoint) {
        // 移动整条线
        QPointF delta = newPos - grip->position;
        line->moveBy(delta.x(), delta.y());
    }
}

void GripManager::dragCircleGrip(CircleItem *circle, GripPoint *grip, const QPointF &newPos)
{
    if (grip->type == Grip_Center) {
        QPointF delta = newPos - grip->position;
        circle->moveBy(delta.x(), delta.y());
    } else if (grip->type == Grip_Quadrant) {
        // 修改半径
        qreal newRadius = QLineF(circle->centerPoint(), newPos).length();
        circle->setRadius(newRadius);
    }
}

void GripManager::dragArcGrip(ArcItem *arc, GripPoint *grip, const QPointF &newPos)
{
    if (grip->type == Grip_Center) {
        QPointF delta = newPos - grip->position;
        arc->moveBy(delta.x(), delta.y());
    } else if (grip->type == Grip_Endpoint) {
        // 修改起点或终点角度
        QPointF c = arc->centerPoint();
        qreal angle = qAtan2(newPos.y() - c.y(), newPos.x() - c.x());
        if (grip->index == 0) {
            arc->setStartAngle(angle);
        } else if (grip->index == 1) {
            arc->setEndAngle(angle);
        }
    } else if (grip->type == Grip_Midpoint) {
        // 修改半径
        qreal newRadius = QLineF(arc->centerPoint(), newPos).length();
        arc->setRadius(newRadius);
    }
}

void GripManager::dragPolylineGrip(PolylineItem *poly, GripPoint *grip, const QPointF &newPos)
{
    if (grip->type == Grip_Vertex && grip->index >= 0) {
        auto points = poly->points();
        if (grip->index < points.size()) {
            points[grip->index] = newPos;
            poly->setPoints(points);
        }
    }
}

void GripManager::dragRectangleGrip(RectangleItem *rect, GripPoint *grip, const QPointF &newPos)
{
    QRectF r = rect->rect();
    if (grip->type == Grip_Vertex) {
        // 修改角点
        switch (grip->index) {
        case 0: r.setTopLeft(newPos); break;
        case 1: r.setTopRight(newPos); break;
        case 2: r.setBottomRight(newPos); break;
        case 3: r.setBottomLeft(newPos); break;
        }
        rect->setRect(r);
    } else if (grip->type == Grip_Midpoint) {
        // 修改边中点（拉伸）
        switch (grip->index) {
        case 4: r.setTop(newPos.y()); break;
        case 5: r.setRight(newPos.x()); break;
        case 6: r.setBottom(newPos.y()); break;
        case 7: r.setLeft(newPos.x()); break;
        }
        rect->setRect(r);
    }
}

void GripManager::drawGrips(QPainter *painter)
{
    if (!m_enabled || m_grips.isEmpty() || !m_view) return;

    for (auto grip : m_grips) {
        QPoint screenPos = m_view->mapFromScene(grip->position);
        qreal size = m_gripSize;

        painter->save();
        painter->translate(screenPos);

        if (grip->isHot) {
            // 热夹点：实心红色
            painter->setBrush(QColor(255, 0, 0));
            painter->setPen(QPen(QColor(255, 0, 0), 1));
        } else {
            // 冷夹点：蓝色边框，白色填充
            painter->setBrush(QColor(255, 255, 255));
            painter->setPen(QPen(QColor(0, 0, 255), 1.5));
        }

        switch (grip->type) {
        case Grip_Endpoint:
        case Grip_Vertex:
            // 方形
            painter->drawRect(QRectF(-size/2, -size/2, size, size));
            break;
        case Grip_Midpoint:
            // 三角形
            painter->drawPolygon(QPolygonF() << QPointF(0, -size/2)
                                             << QPointF(size/2, size/2)
                                             << QPointF(-size/2, size/2));
            break;
        case Grip_Center:
        case Grip_Insertion:
            // 圆形
            painter->drawEllipse(QRectF(-size/2, -size/2, size, size));
            break;
        case Grip_Quadrant:
            // 菱形
            painter->drawPolygon(QPolygonF() << QPointF(0, -size/2)
                                             << QPointF(size/2, 0)
                                             << QPointF(0, size/2)
                                             << QPointF(-size/2, 0));
            break;
        default:
            painter->drawRect(QRectF(-size/2, -size/2, size, size));
        }

        painter->restore();
    }
}

} // namespace Zhifen
