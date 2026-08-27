#include "polylinetool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/polylineitem.h"
#include <QMouseEvent>
#include <QPainter>
PolylineTool::PolylineTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void PolylineTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        if (m_points.size() >= 2 && m_scene) m_scene->addItem(new PolylineItem(m_points, false));
        emit finished(); return;
    }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_view->orthoMode() && !m_points.isEmpty()) {
        QPointF last = m_points.last();
        if (qAbs(wp.x()-last.x()) > qAbs(wp.y()-last.y())) wp.setY(last.y()); else wp.setX(last.x());
    }
    if (!m_hasStart) { m_points.clear(); m_points.append(wp); m_hasStart = true; emit statusMessage("指定下一点或[闭合(C)]:"); }
    else m_points.append(wp);
    m_currentPos = wp;
}
void PolylineTool::mouseMoveEvent(QMouseEvent *event) {
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_view->orthoMode() && !m_points.isEmpty()) {
        QPointF last = m_points.last();
        if (qAbs(wp.x()-last.x()) > qAbs(wp.y()-last.y())) wp.setY(last.y()); else wp.setX(last.x());
    }
    m_currentPos = wp; m_lastWorldPos = wp;
}
void PolylineTool::mouseReleaseEvent(QMouseEvent *) {}
void PolylineTool::keyPressEvent(QKeyEvent *event) {
    if (event->key()==Qt::Key_Escape||event->key()==Qt::Key_Return) {
        if (m_points.size() >= 2 && m_scene) m_scene->addItem(new PolylineItem(m_points, false));
        emit finished(); event->accept();
    } else if (event->key() == Qt::Key_C && m_points.size() >= 3) {
        if (m_scene) m_scene->addItem(new PolylineItem(m_points, true));
        emit finished(); event->accept();
    }
}
void PolylineTool::drawOverlay(QPainter *painter) {
    if (m_hasStart && !m_points.isEmpty()) {
        painter->save(); painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        for (int i=0; i<m_points.size()-1; i++)
            painter->drawLine(m_view->mapFromScene(m_points[i]), m_view->mapFromScene(m_points[i+1]));
        painter->drawLine(m_view->mapFromScene(m_points.last()), m_view->mapFromScene(m_currentPos));
        painter->restore();
    }
}
void PolylineTool::deactivate() { m_points.clear(); m_hasStart = false; }
