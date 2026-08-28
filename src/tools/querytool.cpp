#include "querytool.h"
#include "../cad/cadview.h"
#include "../cad/cadscene.h"
#include "../entities/caditem.h"
#include "../entities/lineitem.h"
#include "../entities/circleitem.h"
#include "../entities/arcitem.h"
#include "../entities/polylineitem.h"
#include "../entities/rectangleitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QMessageBox>

QueryTool::QueryTool(CadView *view, QueryType type, QObject *parent)
    : Tool(view, parent), m_type(type) {}

QString QueryTool::name() const
{
    switch (m_type) {
    case Distance: return "距离查询";
    case Area: return "面积查询";
    case Point: return "坐标查询";
    }
    return "查询";
}

void QueryTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());

    if (m_type == Point) {
        QString result = QString("坐标: X=%1, Y=%2").arg(wp.x(), 0, 'f', 2).arg(wp.y(), 0, 'f', 2);
        emit statusMessage(result);
        emit queryResult(result);
        m_step = 0;
    } else if (m_type == Distance) {
        if (m_step == 0) {
            m_p1 = wp;
            m_step = 1;
            emit statusMessage("指定第二点:");
        } else {
            m_p2 = wp;
            qreal dist = QLineF(m_p1, m_p2).length();
            qreal angle = QLineF(m_p1, m_p2).angle();
            qreal dx = m_p2.x() - m_p1.x();
            qreal dy = m_p2.y() - m_p1.y();
            QString result = QString("距离=%1, 角度=%2°, ΔX=%3, ΔY=%4")
                .arg(dist, 0, 'f', 2).arg(angle, 0, 'f', 2)
                .arg(dx, 0, 'f', 2).arg(dy, 0, 'f', 2);
            emit statusMessage(result);
            emit queryResult(result);
            m_step = 0;
            emit statusMessage("指定第一点:");
        }
    } else if (m_type == Area) {
        QList<QGraphicsItem*> items = m_scene->items(wp);
        for (auto *item : items) {
            CadItem *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            qreal area = 0, perimeter = 0;
            QString shapeName;
            if (auto circle = dynamic_cast<CircleItem*>(cad)) {
                qreal r = circle->radius();
                area = M_PI * r * r;
                perimeter = 2 * M_PI * r;
                shapeName = "圆";
            } else if (auto rect = dynamic_cast<RectangleItem*>(cad)) {
                QRectF r = rect->rectangle();
                area = r.width() * r.height();
                perimeter = 2 * (r.width() + r.height());
                shapeName = "矩形";
            } else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
                if (poly->isClosed() && poly->points().size() >= 3) {
                    QPolygonF pts = poly->points();
                    for (int i = 0; i < pts.size(); i++) {
                        int j = (i + 1) % pts.size();
                        area += pts[i].x() * pts[j].y();
                        area -= pts[j].x() * pts[i].y();
                        perimeter += QLineF(pts[i], pts[j]).length();
                    }
                    area = qAbs(area) / 2.0;
                    shapeName = "闭合多段线";
                }
            }
            if (area > 0) {
                QString result = QString("%1: 面积=%2, 周长=%3")
                    .arg(shapeName).arg(area, 0, 'f', 2).arg(perimeter, 0, 'f', 2);
                emit statusMessage(result);
                emit queryResult(result);
                return;
            }
        }
        emit statusMessage("请选择闭合对象（圆/矩形/闭合多段线）");
    }
    m_currentPos = wp;
}

void QueryTool::mouseMoveEvent(QMouseEvent *event) {
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void QueryTool::mouseReleaseEvent(QMouseEvent *) {}

void QueryTool::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) { m_step = 0; emit finished(); event->accept(); }
}

void QueryTool::drawOverlay(QPainter *painter) {
    if (m_type == Distance && m_step == 1) {
        painter->save();
        painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
        painter->restore();
    }
}

void QueryTool::deactivate() { m_step = 0; }
