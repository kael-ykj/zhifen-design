#include "zoomtool.h"
#include "cadview.h"
#include <QMouseEvent>
#include <QPainter>

ZoomTool::ZoomTool(CadView *view, QObject *parent)
    : Tool(view, parent)
{
}

void ZoomTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_view->zoomOut();
        return;
    }
    if (event->button() != Qt::LeftButton) return;
    m_startPos = event->pos();
    m_currentPos = event->pos();
    m_selecting = true;
}

void ZoomTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentPos = event->pos();
    m_lastWorldPos = m_view->mapToScene(event->pos());
}

void ZoomTool::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) return;
    if (m_selecting) {
        QRectF rect = QRectF(m_startPos, m_currentPos).normalized();
        if (rect.width() > 5 && rect.height() > 5) {
            QRectF worldRect = QRectF(m_view->mapToScene(m_startPos), m_view->mapToScene(m_currentPos)).normalized();
            m_view->fitInView(worldRect, Qt::KeepAspectRatio);
        } else {
            m_view->zoomIn();
        }
        emit finished();
    }
    m_selecting = false;
}

void ZoomTool::drawOverlay(QPainter *painter)
{
    if (m_selecting) {
        QRectF rect = QRectF(m_startPos, m_currentPos).normalized();
        painter->save();
        painter->setPen(QPen(QColor(0, 255, 0), 1, Qt::DashLine));
        painter->drawRect(rect);
        painter->restore();
    }
}
