#include "rotatetool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/caditem.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/arcitem.h"
#include "entities/polylineitem.h"
#include "entities/rectangleitem.h"
#include "entities/textitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
RotateTool::RotateTool(CadView *view, QObject *parent) : Tool(view, parent) {}
QPointF RotateTool::rotatePoint(const QPointF &p, const QPointF &center, qreal angleRad) {
    qreal dx = p.x() - center.x(), dy = p.y() - center.y();
    qreal c = cos(angleRad), s = sin(angleRad);
    return QPointF(center.x() + dx*c - dy*s, center.y() + dx*s + dy*c);
}
void RotateTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_step == 0) { m_basePoint = wp; m_step = 1; if (m_scene) m_selected = m_scene->selectedItems(); emit statusMessage("指定参照角:"); }
    else if (m_step == 1) { m_refPoint = wp; m_step = 2; emit statusMessage("指定新角度:"); }
    else {
        qreal ang1 = atan2(m_refPoint.y()-m_basePoint.y(), m_refPoint.x()-m_basePoint.x());
        qreal ang2 = atan2(wp.y()-m_basePoint.y(), wp.x()-m_basePoint.x());
        qreal delta = ang2 - ang1;
        for (auto item : m_selected) {
            CadItem *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            if (auto line = dynamic_cast<LineItem*>(cad))
                line->setLine(rotatePoint(line->startPoint(),m_basePoint,delta), rotatePoint(line->endPoint(),m_basePoint,delta));
            else if (auto arc = dynamic_cast<ArcItem*>(cad))
                arc->setArc(rotatePoint(arc->centerPoint(),m_basePoint,delta), arc->radius(), arc->startAngle()+delta*180.0/3.14159265358979323846, arc->spanAngle());
            else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
                QPolygonF pts; for (auto p : poly->points()) pts.append(rotatePoint(p,m_basePoint,delta));
                poly->setPolyline(pts, poly->isClosed());
            } else if (auto rect = dynamic_cast<RectangleItem*>(cad)) {
                QPointF tl = rotatePoint(rect->rectangle().topLeft(),m_basePoint,delta);
                QPointF br = rotatePoint(rect->rectangle().bottomRight(),m_basePoint,delta);
                rect->setRectangle(QRectF(tl,br).normalized());
            } else if (auto text = dynamic_cast<TextItem*>(cad))
                text->setPos(rotatePoint(text->pos(),m_basePoint,delta));
        }
        m_step = 0; emit statusMessage("指定基点:");
    }
    m_currentPos = wp;
}
void RotateTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void RotateTool::mouseReleaseEvent(QMouseEvent *) {}
void RotateTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void RotateTool::drawOverlay(QPainter *) {}
void RotateTool::deactivate() { m_step = 0; m_selected.clear(); }
