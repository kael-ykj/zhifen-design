#include "cadview.h"
#include "cadscene.h"
#include "tool.h"
#include "snapmanager.h"
#include "document.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QPainter>
#include <QtMath>

CadView::CadView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setBackgroundBrush(QColor(30, 30, 30));
}

void CadView::setCadScene(CadScene *scene)
{
    setScene(scene);
    m_snapManager = new SnapManager(scene, this);
}

void CadView::zoomExtents()
{
    if (!scene()) return;
    QRectF rect = scene()->itemsBoundingRect();
    if (rect.isEmpty()) rect = QRectF(-100, -100, 200, 200);
    rect.adjust(-rect.width() * 0.1, -rect.height() * 0.1,
                rect.width() * 0.1, rect.height() * 0.1);
    fitInView(rect, Qt::KeepAspectRatio);
}

void CadView::zoomIn()
{
    scale(1.2, 1.2);
}

void CadView::zoomOut()
{
    scale(1 / 1.2, 1 / 1.2);
}

void CadView::zoomAt(const QPoint &screenPos, qreal factor)
{
    QPointF worldPos = mapToScene(screenPos);
    scale(factor, factor);
    QPointF newWorldPos = mapToScene(screenPos);
    QPointF delta = newWorldPos - worldPos;
    translate(delta.x(), delta.y());
}

QPointF CadView::screenToWorld(const QPoint &screenPos) const
{
    return mapToScene(screenPos);
}

QPoint CadView::worldToScreen(const QPointF &worldPos) const
{
    return mapFromScene(worldPos);
}

void CadView::mousePressEvent(QMouseEvent *event)
{
    // 中键平移
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (m_currentTool) {
        m_currentTool->mousePressEvent(event);
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void CadView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF worldPos = mapToScene(event->pos());
    m_lastWorldPos = worldPos;
    emit coordinateChanged(worldPos);

    // 中键平移
    if (m_panning) {
        QPoint delta = event->pos() - m_panStart;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_panStart = event->pos();
        return;
    }

    if (m_currentTool) {
        m_currentTool->mouseMoveEvent(event);
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void CadView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && m_panning) {
        m_panning = false;
        setCursor(m_currentTool ? m_currentTool->cursor() : Qt::CrossCursor);
        return;
    }

    if (m_currentTool) {
        m_currentTool->mouseReleaseEvent(event);
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void CadView::wheelEvent(QWheelEvent *event)
{
    QPoint angle = event->angleDelta();
    qreal factor = angle.y() > 0 ? 1.15 : 1 / 1.15;
    zoomAt(event->pos(), factor);
    event->accept();
}

void CadView::keyPressEvent(QKeyEvent *event)
{
    if (m_currentTool) {
        m_currentTool->keyPressEvent(event);
        if (event->isAccepted()) return;
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        if (m_currentTool) {
            emit toolFinished();
        }
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        // 删除选中项
        if (scene()) {
            for (auto item : scene()->selectedItems()) {
                scene()->removeItem(item);
            }
        }
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void CadView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    // 绘制捕捉标记
    if (m_snapManager && m_snapManager->hasSnap()) {
        m_snapManager->drawSnapMarker(painter);
    }
    // 绘制工具覆盖层
    if (m_currentTool) {
        m_currentTool->drawOverlay(painter);
    }
}
