#include "dimensiontool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/dimensionitem.h"
#include <QMouseEvent>
#include <QPainter>
DimensionTool::DimensionTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void DimensionTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_step == 0) { m_p1 = wp; m_step = 1; emit statusMessage("指定第二条尺寸界线原点:"); }
    else if (m_step == 1) { m_p2 = wp; m_step = 2; emit statusMessage("指定尺寸线位置:"); }
    else {
        m_dimPos = wp;
        if (m_scene) m_scene->addItem(new DimensionItem(DimensionItem::Linear, m_p1, m_p2, m_dimPos));
        m_step = 0;
        emit statusMessage("指定第一条尺寸界线原点:");
    }
    m_currentPos = wp;
}
void DimensionTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void DimensionTool::mouseReleaseEvent(QMouseEvent *) {}
void DimensionTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void DimensionTool::drawOverlay(QPainter *painter) {
    if (m_step >= 1) {
        painter->save(); painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        if (m_step == 1) painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
        else if (m_step == 2) {
            painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_p2));
            painter->drawLine(m_view->mapFromScene(m_p2), m_view->mapFromScene(m_currentPos));
        }
        painter->restore();
    }
}
void DimensionTool::deactivate() { m_step = 0; }
