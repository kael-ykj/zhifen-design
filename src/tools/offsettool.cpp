#include "offsettool.h"
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

OffsetTool::OffsetTool(CadView *view, QObject *parent) : Tool(view, parent) {}

void OffsetTool::offsetItem(QGraphicsItem *item, const QPointF &sidePoint)
{
    CadItem *cad = dynamic_cast<CadItem*>(item);
    if (!cad) return;

    if (auto line = dynamic_cast<LineItem*>(cad)) {
        QPointF p1 = line->startPoint(), p2 = line->endPoint();
        QPointF d = p2 - p1;
        qreal len = sqrt(d.x()*d.x() + d.y()*d.y());
        if (len < 1e-10) return;
        QPointF normal(-d.y()/len, d.x()/len);
        // 判断侧点在哪一侧
        QPointF toSide = sidePoint - p1;
        qreal dot = toSide.x()*normal.x() + toSide.y()*normal.y();
        QPointF offset = normal * (dot > 0 ? m_distance : -m_distance);
        LineItem *newLine = new LineItem(p1 + offset, p2 + offset);
        newLine->setLayer(cad->layer());
        newLine->setColor(cad->color());
        newLine->setColorByLayer(cad->isColorByLayer());
        m_scene->addItem(newLine);
        emit statusMessage("直线已偏移");
    } else if (auto circle = dynamic_cast<CircleItem*>(cad)) {
        QPointF c = circle->centerPoint();
        qreal r = circle->radius();
        qreal distToCenter = QLineF(c, sidePoint).length();
        qreal newR = distToCenter > r ? r + m_distance : r - m_distance;
        if (newR < 0.1) newR = 0.1;
        CircleItem *newCircle = new CircleItem(c, newR);
        newCircle->setLayer(cad->layer());
        newCircle->setColor(cad->color());
        newCircle->setColorByLayer(cad->isColorByLayer());
        m_scene->addItem(newCircle);
        emit statusMessage("圆已偏移");
    } else if (auto arc = dynamic_cast<ArcItem*>(cad)) {
        QPointF c = arc->centerPoint();
        qreal r = arc->radius();
        qreal distToCenter = QLineF(c, sidePoint).length();
        qreal newR = distToCenter > r ? r + m_distance : r - m_distance;
        if (newR < 0.1) newR = 0.1;
        ArcItem *newArc = new ArcItem(c, newR, arc->startAngle(), arc->spanAngle());
        newArc->setLayer(cad->layer());
        newArc->setColor(cad->color());
        newArc->setColorByLayer(cad->isColorByLayer());
        m_scene->addItem(newArc);
        emit statusMessage("圆弧已偏移");
    } else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
        QPolygonF pts = poly->points();
        if (pts.size() < 2) return;
        QPolygonF newPts;
        for (int i = 0; i < pts.size(); i++) {
            QPointF prev = pts[i > 0 ? i-1 : 0];
            QPointF curr = pts[i];
            QPointF next = pts[i < pts.size()-1 ? i+1 : pts.size()-1];
            QPointF d1 = curr - prev;
            QPointF d2 = next - curr;
            qreal l1 = sqrt(d1.x()*d1.x() + d1.y()*d1.y());
            qreal l2 = sqrt(d2.x()*d2.x() + d2.y()*d2.y());
            QPointF n1 = l1 > 1e-10 ? QPointF(-d1.y()/l1, d1.x()/l1) : QPointF(0,0);
            QPointF n2 = l2 > 1e-10 ? QPointF(-d2.y()/l2, d2.x()/l2) : QPointF(0,0);
            QPointF avgN = (n1 + n2) * 0.5;
            qreal nl = sqrt(avgN.x()*avgN.x() + avgN.y()*avgN.y());
            if (nl > 1e-10) avgN = avgN / nl;
            QPointF toSide = sidePoint - curr;
            qreal dot = toSide.x()*avgN.x() + toSide.y()*avgN.y();
            newPts.append(curr + avgN * (dot > 0 ? m_distance : -m_distance));
        }
        PolylineItem *newPoly = new PolylineItem(newPts, poly->isClosed());
        newPoly->setLayer(cad->layer());
        newPoly->setColor(cad->color());
        newPoly->setColorByLayer(cad->isColorByLayer());
        m_scene->addItem(newPoly);
        emit statusMessage("多段线已偏移");
    } else {
        emit statusMessage("该对象无法偏移");
    }
}

void OffsetTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());

    if (m_step == 0) {
        m_p1 = wp;
        m_step = 1;
        emit statusMessage("指定第二点:");
    } else if (m_step == 1) {
        m_distance = QLineF(m_p1, wp).length();
        if (m_distance < 0.1) m_distance = 10.0;
        m_step = 2;
        emit statusMessage(QString("偏移距离=%1，选择要偏移的对象:").arg(m_distance, 0, 'f', 2));
    } else if (m_step == 2) {
        QList<QGraphicsItem*> items = m_scene->items(wp);
        if (!items.isEmpty()) {
            m_selected = items.first();
            m_step = 3;
            emit statusMessage("指定偏移侧:");
        }
    } else if (m_step == 3) {
        if (m_selected) {
            offsetItem(m_selected, wp);
        }
        m_selected = nullptr;
        m_step = 2;
        emit statusMessage("选择要偏移的对象或[退出(E)]:");
    }
    m_currentPos = wp;
}

void OffsetTool::mouseMoveEvent(QMouseEvent *event) {
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void OffsetTool::mouseReleaseEvent(QMouseEvent *) {}

void OffsetTool::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_step > 0) { m_step = 0; m_selected = nullptr; emit statusMessage("指定偏移距离第一点:"); }
        else emit finished();
        event->accept();
    }
}

void OffsetTool::drawOverlay(QPainter *painter) {
    if (m_step == 1) {
        painter->save();
        painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
        painter->restore();
    }
}

void OffsetTool::deactivate() { m_step = 0; m_selected = nullptr; }
