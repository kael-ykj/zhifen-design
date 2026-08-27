#include "copytool.h"
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
CopyTool::CopyTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void CopyTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (!m_hasBase) {
        m_basePoint = wp; m_hasBase = true;
        if (m_scene) m_selected = m_scene->selectedItems();
        emit statusMessage("指定第二个点:");
    } else {
        QPointF delta = wp - m_basePoint;
        for (auto item : m_selected) {
            CadItem *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            CadItem *copy = nullptr;
            if (auto line = dynamic_cast<LineItem*>(cad))
                copy = new LineItem(line->startPoint()+delta, line->endPoint()+delta);
            else if (auto circle = dynamic_cast<CircleItem*>(cad))
                copy = new CircleItem(circle->centerPoint()+delta, circle->radius());
            else if (auto arc = dynamic_cast<ArcItem*>(cad))
                copy = new ArcItem(arc->centerPoint()+delta, arc->radius(), arc->startAngle(), arc->spanAngle());
            else if (auto poly = dynamic_cast<PolylineItem*>(cad)) {
                QPolygonF pts; for (auto p : poly->points()) pts.append(p+delta);
                copy = new PolylineItem(pts, poly->isClosed());
            } else if (auto rect = dynamic_cast<RectangleItem*>(cad))
                copy = new RectangleItem(QRectF(rect->rectangle().topLeft()+delta, rect->rectangle().bottomRight()+delta));
            else if (auto text = dynamic_cast<TextItem*>(cad))
                copy = new TextItem(text->position()+delta, text->text(), text->textHeight());
            if (copy) { copy->setLayer(cad->layer()); copy->setColor(cad->color()); copy->setColorByLayer(cad->isColorByLayer()); m_scene->addItem(copy); }
        }
        emit statusMessage("指定第二个点或[退出(E)]:");
    }
    m_currentPos = wp;
}
void CopyTool::mouseMoveEvent(QMouseEvent *event) { m_currentPos = m_view->mapToScene(event->pos()); m_lastWorldPos = m_currentPos; }
void CopyTool::mouseReleaseEvent(QMouseEvent *) {}
void CopyTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void CopyTool::drawOverlay(QPainter *) {}
void CopyTool::deactivate() { m_hasBase = false; m_selected.clear(); }
