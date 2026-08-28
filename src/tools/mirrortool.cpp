#include "mirrortool.h"
#include "../cad/cadview.h"
#include "../cad/cadscene.h"
#include "../entities/caditem.h"
#include "../entities/lineitem.h"
#include "../entities/circleitem.h"
#include "../entities/arcitem.h"
#include "../entities/polylineitem.h"
#include "../entities/rectangleitem.h"
#include "../entities/textitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

MirrorTool::MirrorTool(CadView *view, QObject *parent) : Tool(view, parent) {}

static QPointF mirrorPoint(const QPointF &p, const QPointF &p1, const QPointF &p2)
{
    QPointF d = p2 - p1;
    qreal len2 = d.x()*d.x() + d.y()*d.y();
    if (len2 < 1e-10) return p;
    qreal t = ((p.x()-p1.x())*d.x() + (p.y()-p1.y())*d.y()) / len2;
    QPointF proj = p1 + d * t;
    return proj * 2 - p;
}

void MirrorTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());

    if (m_step == 0) {
        m_p1 = wp;
        m_step = 1;
        if (m_scene) m_selected = m_scene->selectedItems();
        emit statusMessage("指定镜像线第二点:");
    } else if (m_step == 1) {
        m_p2 = wp;
        m_step = 2;
        emit statusMessage("要删除源对象吗？[是(Y)/否(N)] <N>:");
    } else {
        // 执行镜像
        for (auto item : m_selected) {
            CadItem *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            CadItem *mirrored = nullptr;
            if (auto line = dynamic_cast<LineItem*>(cad))
                mirrored = new LineItem(mirrorPoint(line->startPoint(), m_p1, m_p2),
                                        mirrorPoint(line->endPoint(), m_p1, m_p2));
            else if (auto circle = dynamic_cast<CircleItem*>(cad))
                mirrored = new CircleItem(mirrorPoint(circle->centerPoint(), m_p1, m_p2), circle->radius());
            else if (auto arc = dynamic_cast<ArcItem*>(cad)) {
                QPointF c = mirrorPoint(arc->centerPoint(), m_p1, m_p2);
                mirrored = new ArcItem(c, arc->radius(), -arc->startAngle() - arc->spanAngle(), arc->spanAngle());
            } else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
                QPolygonF pts;
                for (auto p : poly->points()) pts.append(mirrorPoint(p, m_p1, m_p2));
                mirrored = new PolylineItem(pts, poly->isClosed());
            } else if (auto rect = dynamic_cast<RectangleItem*>(cad)) {
                QRectF r = rect->rectangle();
                QPointF tl = mirrorPoint(r.topLeft(), m_p1, m_p2);
                QPointF br = mirrorPoint(r.bottomRight(), m_p1, m_p2);
                mirrored = new RectangleItem(QRectF(tl, br).normalized());
            } else if (auto text = dynamic_cast<TextItem*>(cad))
                mirrored = new TextItem(mirrorPoint(text->position(), m_p1, m_p2), text->text(), text->textHeight());
            if (mirrored) {
                mirrored->setLayer(cad->layer());
                mirrored->setColor(cad->color());
                mirrored->setColorByLayer(cad->isColorByLayer());
                m_scene->addItem(mirrored);
            }
        }
        if (m_deleteSource) {
            for (auto item : m_selected) m_scene->removeItem(item);
        }
        m_step = 0;
        m_selected.clear();
        emit statusMessage("指定镜像线第一点:");
    }
    m_currentPos = wp;
}

void MirrorTool::mouseMoveEvent(QMouseEvent *event) {
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void MirrorTool::mouseReleaseEvent(QMouseEvent *) {}

void MirrorTool::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) { emit finished(); event->accept(); }
    else if (m_step == 2 && event->key() == Qt::Key_Y) { m_deleteSource = true; }
    else if (m_step == 2 && event->key() == Qt::Key_N) { m_deleteSource = false; }
}

void MirrorTool::drawOverlay(QPainter *painter) {
    if (m_step >= 1) {
        painter->save();
        painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
        painter->restore();
    }
}

void MirrorTool::deactivate() { m_step = 0; m_selected.clear(); }
