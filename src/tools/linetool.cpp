#include "linetool.h"
#include "cadview.h"
#include "cadscene.h"
#include "lineitem.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

LineTool::LineTool(CadView *view, QObject *parent)
    : Tool(view, parent)
{
}

void LineTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit finished();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    QPointF worldPos = m_view->mapToScene(event->pos());

    // 正交模式
    if (m_view->orthoMode() && m_hasStart) {
        QPointF last = m_points.last();
        if (qAbs(worldPos.x() - last.x()) > qAbs(worldPos.y() - last.y())) {
            worldPos.setY(last.y());
        } else {
            worldPos.setX(last.x());
        }
    }

    if (!m_hasStart) {
        m_points.clear();
        m_points.append(worldPos);
        m_hasStart = true;
        emit statusMessage("指定下一点:");
    } else {
        m_points.append(worldPos);
        // 创建直线图元
        if (m_scene && m_points.size() >= 2) {
            LineItem *line = new LineItem(m_points[m_points.size() - 2], m_points.last());
            line->setLayer("0");
            m_scene->addItem(line);
        }
    }
    m_currentPos = worldPos;
}

void LineTool::mouseMoveEvent(QMouseEvent *event)
{
    QPointF worldPos = m_view->mapToScene(event->pos());
    if (m_view->orthoMode() && m_hasStart) {
        QPointF last = m_points.last();
        if (qAbs(worldPos.x() - last.x()) > qAbs(worldPos.y() - last.y())) {
            worldPos.setY(last.y());
        } else {
            worldPos.setX(last.x());
        }
    }
    m_currentPos = worldPos;
    m_lastWorldPos = worldPos;
}

void LineTool::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void LineTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Return) {
        emit finished();
        event->accept();
    }
}

void LineTool::drawOverlay(QPainter *painter)
{
    if (m_hasStart && !m_points.isEmpty()) {
        painter->save();
        painter->setPen(QPen(QColor(255, 255, 0), 1, Qt::DashLine));
        QPointF last = m_points.last();
        painter->drawLine(last, m_currentPos);
        // 显示长度和角度
        qreal len = QLineF(last, m_currentPos).length();
        qreal ang = (QLineF(last, m_currentPos * 180.0 / 3.14159265358979323846).angle());
        QPoint screenPos = m_view->mapFromScene(m_currentPos);
        painter->setPen(QColor(255, 255, 0));
        painter->setFont(QFont("Arial", 10));
        painter->drawText(screenPos + QPoint(10, -10), QString("长度: %1  角度: %2°").arg(len, 0, 'f', 2).arg(ang, 0, 'f', 1));
        painter->restore();
    }
}

void LineTool::deactivate()
{
    m_points.clear();
    m_hasStart = false;
}
