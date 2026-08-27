#include "pantool.h"
#include "cadview.h"
#include <QMouseEvent>
#include <QScrollBar>

PanTool::PanTool(CadView *view, QObject *parent)
    : Tool(view, parent)
{
}

void PanTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPos = event->pos();
        m_view->setCursor(Qt::ClosedHandCursor);
    }
}

void PanTool::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPos;
        m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->value() - delta.x());
        m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->value() - delta.y());
        m_lastPos = event->pos();
    }
    m_lastWorldPos = m_view->mapToScene(event->pos());
}

void PanTool::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton) {
        m_panning = false;
        m_view->setCursor(Qt::OpenHandCursor);
    }
}
