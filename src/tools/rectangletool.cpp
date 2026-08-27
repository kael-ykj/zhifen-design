#include "rectangletool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/rectangleitem.h"
#include <QMouseEvent>
#include <QPainter>
RectangleTool::RectangleTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void RectangleTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (!m_hasStart) { m_p1 = wp; m_hasStart = true; emit statusMessage("指定另一个角点:"); }
    else {
        QPointF p2 = wp;
        if (m_view->orthoMode()) {
            qreal w = qAbs(p2.x() - m_p1.x());
            qreal h = qAbs(p2.y() - m_p1.y());
            qreal s = qMax(w, h);
            p2 = QPointF(m_p1.x() + (p2.x()>m_p1.x()?s:-s), m_p1.y() + (p2.y()>m_p1.y()?s:-s));
        }
        if (m_scene) m_scene->addItem(new RectangleItem(QRectF(m_p1, p2).normalized()));
        m_hasStart = false;
        emit statusMessage("指定第一个角点:");
    }
    m_currentPos = wp;
}
void RectangleTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void RectangleTool::mouseReleaseEvent(QMouseEvent *) {}
void RectangleTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void RectangleTool::drawOverlay(QPainter *painter) {
    if (m_hasStart) {
        painter->save(); painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        QRectF r = QRectF(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos)).normalized();
        painter->drawRect(r);
        painter->restore();
    }
}
void RectangleTool::deactivate() { m_hasStart = false; }
