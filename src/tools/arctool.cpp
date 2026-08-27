#include "arctool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/arcitem.h"
#include <QMouseEvent>
#include <QPainter>
ArcTool::ArcTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void ArcTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_step == 0) { m_p1 = wp; m_step = 1; emit statusMessage("指定圆弧第二点:"); }
    else if (m_step == 1) { m_p2 = wp; m_step = 2; emit statusMessage("指定圆弧端点:"); }
    else {
        m_p3 = wp;
        QPointF center; qreal r, sa, span;
        // 三点求圆心
        qreal ax=m_p1.x(), ay=m_p1.y(), bx=m_p2.x(), by=m_p2.y(), cx=m_p3.x(), cy=m_p3.y();
        qreal d = 2*(ax*(by-cy)+bx*(cy-ay)+cx*(ay-by));
        if (qAbs(d) < 1e-9) { emit statusMessage("三点共线，无法画弧"); return; }
        qreal ux = ((ax*ax+ay*ay)*(by-cy)+(bx*bx+by*by)*(cy-ay)+(cx*cx+cy*cy)*(ay-by))/d;
        qreal uy = ((ax*ax+ay*ay)*(cx-bx)+(bx*bx+by*by)*(ax-cx)+(cx*cx+cy*cy)*(bx-ax))/d;
        center = QPointF(ux, uy);
        r = QLineF(center, m_p1).length();
        sa = QLineF(center, m_p1).angle();
        qreal ea = QLineF(center, m_p3).angle();
        span = ea - sa; if (span < 0) span += 360;
        if (m_scene) m_scene->addItem(new ArcItem(center, r, sa, span));
        m_step = 0;
        emit statusMessage("指定圆弧起点:");
    }
    m_currentPos = wp;
}
void ArcTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void ArcTool::mouseReleaseEvent(QMouseEvent *) {}
void ArcTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape||event->key()==Qt::Key_Return){emit finished();event->accept();} }
void ArcTool::drawOverlay(QPainter *painter) {
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
void ArcTool::deactivate() { m_step = 0; }
