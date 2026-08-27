#include "snapmanager.h"
#include "cadscene.h"
#include "cadview.h"
#include "caditem.h"
#include "lineitem.h"
#include "circleitem.h"
#include "arcitem.h"
#include "polylineitem.h"
#include <QGraphicsItem>
#include <QtMath>

SnapManager::SnapManager(CadScene *scene, CadView *view, QObject *parent)
    : QObject(parent), m_scene(scene), m_view(view)
{
}

SnapResult* SnapManager::computeSnap(const QPointF &worldPos)
{
    clearSnap();
    if (!m_enabled || !m_scene) return nullptr;

    qreal worldTolerance = m_tolerance / m_view->transform().m11();
    qreal bestDist = worldTolerance;

    for (auto item : m_scene->items()) {
        CadItem *cadItem = dynamic_cast<CadItem*>(item);
        if (!cadItem) continue;

        // 端点捕捉
        if (m_types.contains(SnapEndpoint)) {
            QList<QPointF> endpoints;
            if (auto line = dynamic_cast<LineItem*>(cadItem)) {
                endpoints << line->startPoint() << line->endPoint();
            } else if (auto arc = dynamic_cast<ArcItem*>(cadItem)) {
                endpoints << arc->startPoint() << arc->endPoint();
            } else if (auto poly = dynamic_cast<PolylineItem*>(cadItem)) {
                for (auto p : poly->points()) endpoints << p;
            }
            for (auto ep : endpoints) {
                qreal d = QLineF(worldPos, ep).length();
                if (d < bestDist) {
                    bestDist = d;
                    m_currentSnap = new SnapResult{ep, SnapEndpoint, cadItem};
                }
            }
        }

        // 中点捕捉
        if (m_types.contains(SnapMidpoint)) {
            if (auto line = dynamic_cast<LineItem*>(cadItem)) {
                QPointF mid = (line->startPoint() + line->endPoint()) / 2;
                qreal d = QLineF(worldPos, mid).length();
                if (d < bestDist) {
                    bestDist = d;
                    m_currentSnap = new SnapResult{mid, SnapMidpoint, cadItem};
                }
            }
        }

        // 圆心捕捉
        if (m_types.contains(SnapCenter)) {
            if (auto circle = dynamic_cast<CircleItem*>(cadItem)) {
                QPointF c = circle->centerPoint();
                qreal d = QLineF(worldPos, c).length();
                if (d < bestDist) {
                    bestDist = d;
                    m_currentSnap = new SnapResult{c, SnapCenter, cadItem};
                }
            } else if (auto arc = dynamic_cast<ArcItem*>(cadItem)) {
                QPointF c = arc->centerPoint();
                qreal d = QLineF(worldPos, c).length();
                if (d < bestDist) {
                    bestDist = d;
                    m_currentSnap = new SnapResult{c, SnapCenter, cadItem};
                }
            }
        }
    }

    return m_currentSnap;
}

void SnapManager::drawSnapMarker(QPainter *painter)
{
    if (!m_currentSnap) return;
    QPoint screenPos = m_view->mapFromScene(m_currentSnap->point);
    qreal size = 8;
    painter->save();
    painter->setPen(QPen(QColor(255, 0, 255), 1.5));
    painter->setBrush(Qt::NoBrush);
    painter->translate(screenPos);

    switch (m_currentSnap->type) {
    case SnapEndpoint:
        painter->drawRect(QRectF(-size/2, -size/2, size, size));
        break;
    case SnapMidpoint:
        painter->drawLine(0, -size/2, size/2, 0);
        painter->drawLine(size/2, 0, 0, size/2);
        painter->drawLine(0, size/2, -size/2, 0);
        painter->drawLine(-size/2, 0, 0, -size/2);
        break;
    case SnapCenter:
        painter->drawEllipse(QRectF(-size/2, -size/2, size, size));
        painter->drawLine(-size, 0, size, 0);
        painter->drawLine(0, -size, 0, size);
        break;
    case SnapIntersection:
        painter->drawLine(-size, -size, size, size);
        painter->drawLine(size, -size, -size, size);
        break;
    default:
        painter->drawEllipse(QRectF(-size/4, -size/4, size/2, size/2));
    }
    painter->restore();
}
