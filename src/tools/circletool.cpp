#include "circletool.h"
#include "cadview.h"
#include "cadscene.h"
#include "circleitem.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

CircleTool::CircleTool(CadView *view, QObject *parent)
    : Tool(view, parent)
{
}

void CircleTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit finished();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    QPointF worldPos = m_view->mapToScene(event->pos());

    if (!m_hasCenter) {
        m_center = worldPos;
        m_hasCenter = true;
        emit statusMessage("指定圆的半径:");
    } else {
        qreal radius = QLineF(m_center, worldPos).length();
        if (radius > 0 && m_scene) {
            CircleItem *circle = new CircleItem(m_center, radius);
            circle->setLayer("0");
            m_scene->addItem(circle);
        }
        m_hasCenter = false;
        emit statusMessage("指定圆心:");
    }
    m_currentPos = worldPos;
}

void CircleTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void CircleTool::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void CircleTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return) {
        emit finished();
        event->accept();
    }
}

void CircleTool::drawOverlay(QPainter *painter)
{
    if (m_hasCenter) {
        painter->save();
        painter->setPen(QPen(QColor(255, 255, 0), 1, Qt::DashLine));
        qreal radius = QLineF(m_center, m_currentPos).length();
        QPointF centerScreen = m_view->mapFromScene(m_center);
        qreal r = radius * m_view->transform().m11();
        painter->drawEllipse(centerScreen, r, r);
        painter->setPen(QColor(255, 255, 0));
        painter->setFont(QFont("Arial", 10));
        QPoint curScreen = m_view->mapFromScene(m_currentPos);
        painter->drawText(curScreen + QPoint(10, -10), QString("半径: %1").arg(radius, 0, 'f', 2));
        painter->restore();
    }
}

void CircleTool::deactivate()
{
    m_hasCenter = false;
}
