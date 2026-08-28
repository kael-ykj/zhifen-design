#include "dimensiontool.h"
#include "../cad/cadview.h"
#include "../cad/cadscene.h"
#include "../entities/dimensionitem.h"
#include "../entities/circleitem.h"
#include "../entities/arcitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QUndoStack>
#include "../commands/addentitycommand.h"

DimensionTool::DimensionTool(CadView *view, DimensionItem::DimType type, QObject *parent)
    : Tool(view, parent), m_dimType(type)
{
}

QString DimensionTool::name() const
{
    switch (m_dimType) {
    case DimensionItem::Linear: return "线性标注";
    case DimensionItem::Aligned: return "对齐标注";
    case DimensionItem::Radius: return "半径标注";
    case DimensionItem::Diameter: return "直径标注";
    case DimensionItem::Angular: return "角度标注";
    }
    return "标注";
}

void DimensionTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF scenePos = m_view->mapToScene(event->pos());

    if (m_dimType == DimensionItem::Radius || m_dimType == DimensionItem::Diameter) {
        if (m_step == 0) {
            QList<QGraphicsItem*> items = m_view->scene()->items(scenePos);
            bool found = false;
            for (auto *item : items) {
                auto *circle = dynamic_cast<CircleItem*>(item);
                if (circle) {
                    m_p1 = circle->center();
                    m_p2 = circle->center() + QPointF(circle->radius(), 0);
                    found = true;
                    break;
                }
                auto *arc = dynamic_cast<ArcItem*>(item);
                if (arc) {
                    m_p1 = arc->center();
                    m_p2 = arc->center() + QPointF(arc->radius(), 0);
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_p1 = scenePos;
                m_p2 = scenePos + QPointF(10, 0);
            }
            m_step = 1;
            emit statusMessage("指定标注文字位置:");
        } else {
            m_dimPos = scenePos;
            auto *dim = new DimensionItem(m_dimType, m_p1, m_p2, m_dimPos);
            m_view->scene()->addItem(dim);
            if (m_view->undoStack())
                m_view->undoStack()->push(new AddEntityCommand(dim, m_view->scene()));
            m_step = 0;
            emit statusMessage("选择圆或圆弧:");
        }
    } else if (m_dimType == DimensionItem::Angular) {
        if (m_step == 0) {
            m_p1 = scenePos;
            m_step = 1;
            emit statusMessage("指定第一条边:");
        } else if (m_step == 1) {
            m_p2 = scenePos;
            m_step = 2;
            emit statusMessage("指定第二条边:");
        } else {
            m_dimPos = scenePos;
            auto *dim = new DimensionItem(m_dimType, m_p1, m_p2, m_dimPos);
            m_view->scene()->addItem(dim);
            if (m_view->undoStack())
                m_view->undoStack()->push(new AddEntityCommand(dim, m_view->scene()));
            m_step = 0;
            emit statusMessage("指定角顶点:");
        }
    } else {
        if (m_step == 0) {
            m_p1 = scenePos;
            m_step = 1;
            emit statusMessage("指定第二条尺寸界线原点:");
        } else if (m_step == 1) {
            m_p2 = scenePos;
            m_step = 2;
            emit statusMessage("指定尺寸线位置:");
        } else {
            m_dimPos = scenePos;
            auto *dim = new DimensionItem(m_dimType, m_p1, m_p2, m_dimPos);
            m_view->scene()->addItem(dim);
            if (m_view->undoStack())
                m_view->undoStack()->push(new AddEntityCommand(dim, m_view->scene()));
            m_step = 0;
            emit statusMessage("指定第一条尺寸界线原点:");
        }
    }
    m_currentPos = scenePos;
}

void DimensionTool::mouseMoveEvent(QMouseEvent *event) {
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void DimensionTool::mouseReleaseEvent(QMouseEvent *) {}

void DimensionTool::keyPressEvent(QKeyEvent *event) {
    if (event->key()==Qt::Key_Escape){
        m_step = 0;
        emit finished();
        event->accept();
    }
}

void DimensionTool::drawOverlay(QPainter *painter) {
    if (m_step >= 1) {
        painter->save();
        painter->setPen(QPen(QColor(255,255,0),1,Qt::DashLine));
        if (m_dimType == DimensionItem::Radius || m_dimType == DimensionItem::Diameter) {
            painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
        } else if (m_dimType == DimensionItem::Angular) {
            painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
            if (m_step >= 2) painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_p2));
        } else {
            if (m_step == 1) painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_currentPos));
            else if (m_step == 2) {
                painter->drawLine(m_view->mapFromScene(m_p1), m_view->mapFromScene(m_p2));
                painter->drawLine(m_view->mapFromScene(QPointF(m_p1.x(), m_currentPos.y())),
                                   m_view->mapFromScene(QPointF(m_p2.x(), m_currentPos.y())));
            }
        }
        painter->restore();
    }
}

void DimensionTool::deactivate() { m_step = 0; }
