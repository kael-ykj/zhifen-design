#include "scaletool.h"
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
ScaleTool::ScaleTool(CadView *view, QObject *parent) : Tool(view, parent) {}
QPointF ScaleTool::scalePoint(const QPointF &p, const QPointF &center, qreal factor) {
    return QPointF(center.x() + (p.x()-center.x())*factor, center.y() + (p.y()-center.y())*factor);
}
void ScaleTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_step == 0) { m_basePoint = wp; m_step = 1; if (m_scene) m_selected = m_scene->selectedItems(); emit statusMessage("指定参照长度:"); }
    else if (m_step == 1) { m_refPoint = wp; m_step = 2; emit statusMessage("指定新长度:"); }
    else {
        qreal refLen = QLineF(m_basePoint, m_refPoint).length();
        qreal newLen = QLineF(m_basePoint, wp).length();
        qreal factor = (refLen > 1e-9) ? newLen/refLen : 1.0;
        for (auto item : m_selected) {
            CadItem *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            if (auto line = dynamic_cast<LineItem*>(cad))
                line->setLine(scalePoint(line->startPoint(),m_basePoint,factor), scalePoint(line->endPoint(),m_basePoint,factor));
            else if (auto circle = dynamic_cast<CircleItem*>(cad))
                circle->setCircle(scalePoint(circle->centerPoint(),m_basePoint,factor), circle->radius()*factor);
            else if (auto arc = dynamic_cast<ArcItem*>(cad))
                arc->setArc(scalePoint(arc->centerPoint(),m_basePoint,factor), arc->radius()*factor, arc->startAngle(), arc->spanAngle());
            else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
                QPolygonF pts; for (auto p : poly->points()) pts.append(scalePoint(p,m_basePoint,factor));
                poly->setPolyline(pts, poly->isClosed());
            } else if (auto rect = dynamic_cast<RectangleItem*>(cad)) {
                QPointF tl = scalePoint(rect->rectangle().topLeft(),m_basePoint,factor);
                QPointF br = scalePoint(rect->rectangle().bottomRight(),m_basePoint,factor);
                rect->setRectangle(QRectF(tl,br).normalized());
            } else if (auto text = dynamic_cast<TextItem*>(cad)) {
                text->setPos(scalePoint(text->pos(),m_basePoint,factor));
            }
        }
        m_step = 0; emit statusMessage("指定基点:");
    }
    m_currentPos = wp;
}
void ScaleTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void ScaleTool::mouseReleaseEvent(QMouseEvent *) {}
void ScaleTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void ScaleTool::drawOverlay(QPainter *) {}
void ScaleTool::deactivate() { m_step = 0; m_selected.clear(); }
