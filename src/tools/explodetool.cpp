#include "explodetool.h"
#include "../cad/cadview.h"
#include "../cad/cadscene.h"
#include "../entities/caditem.h"
#include "../entities/lineitem.h"
#include "../entities/polylineitem.h"
#include "../entities/rectangleitem.h"
#include <QMouseEvent>
#include <QPainter>

ExplodeTool::ExplodeTool(CadView *view, QObject *parent) : Tool(view, parent) {}

void ExplodeTool::explodeItem(QGraphicsItem *item)
{
    CadItem *cad = dynamic_cast<CadItem*>(item);
    if (!cad) return;

    if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
        QPolygonF pts = poly->points();
        if (pts.size() < 2) return;
        for (int i = 0; i < pts.size() - 1; i++) {
            LineItem *line = new LineItem(pts[i], pts[i+1]);
            line->setLayer(poly->layer());
            line->setColor(poly->color());
            line->setColorByLayer(poly->isColorByLayer());
            m_scene->addItem(line);
        }
        if (poly->isClosed() && pts.size() > 2) {
            LineItem *line = new LineItem(pts.last(), pts.first());
            line->setLayer(poly->layer());
            line->setColor(poly->color());
            line->setColorByLayer(poly->isColorByLayer());
            m_scene->addItem(line);
        }
        m_scene->removeItem(poly);
        emit statusMessage("多段线已分解为直线");
    } else if (auto rect = dynamic_cast<RectangleItem*>(cad)) {
        QRectF r = rect->rectangle();
        QPointF tl = r.topLeft(), tr = r.topRight(), bl = r.bottomLeft(), br = r.bottomRight();
        QList<QPair<QPointF,QPointF>> edges = {{tl,tr}, {tr,br}, {br,bl}, {bl,tl}};
        for (auto &e : edges) {
            LineItem *line = new LineItem(e.first, e.second);
            line->setLayer(rect->layer());
            line->setColor(rect->color());
            line->setColorByLayer(rect->isColorByLayer());
            m_scene->addItem(line);
        }
        m_scene->removeItem(rect);
        emit statusMessage("矩形已分解为4条直线");
    } else {
        emit statusMessage("该对象无法分解");
    }
}

void ExplodeTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    QList<QGraphicsItem*> items = m_scene->items(wp);
    if (!items.isEmpty()) {
        explodeItem(items.first());
    }
    m_currentPos = wp;
}

void ExplodeTool::mouseMoveEvent(QMouseEvent *event) {
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void ExplodeTool::mouseReleaseEvent(QMouseEvent *) {}

void ExplodeTool::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) { emit finished(); event->accept(); }
}

void ExplodeTool::drawOverlay(QPainter *) {}

void ExplodeTool::deactivate() {}
