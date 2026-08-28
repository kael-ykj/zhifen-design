#include "feedertool.h"
#include "../cad/cadview.h"
#include "../cad/cadscene.h"
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>

FeederTool::FeederTool(CadView *view, QObject *parent) : Tool(view, parent) {}

void FeederTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // 右键结束绘制
        if (m_drawing && m_points.size() >= 2) {
            FeederItem *feeder = new FeederItem(m_points, m_feederType);
            feeder->setLayer("馈线");
            m_scene->addItem(feeder);
        }
        m_points.clear();
        m_drawing = false;
        emit finished();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    QPointF wp = m_view->mapToScene(event->pos());

    // 正交模式
    if (m_view->isOrthoEnabled() && m_points.size() > 0) {
        QPointF last = m_points.last();
        qreal dx = qAbs(wp.x() - last.x());
        qreal dy = qAbs(wp.y() - last.y());
        if (dx > dy) wp.setY(last.y());
        else wp.setX(last.x());
    }

    if (!m_drawing) {
        m_points.clear();
        m_points.append(wp);
        m_drawing = true;
        emit statusMessage("指定下一点或[放弃(U)]:");
    } else {
        m_points.append(wp);
        emit statusMessage(QString("已绘制%1段，指定下一点或[闭合(C)/放弃(U)]:").arg(m_points.size()-1));
    }
    m_currentPos = wp;
}

void FeederTool::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // 双击结束绘制
    if (m_drawing && m_points.size() >= 2) {
        FeederItem *feeder = new FeederItem(m_points, m_feederType);
        feeder->setLayer("馈线");
        m_scene->addItem(feeder);
        emit statusMessage(QString("馈线绘制完成，长度=%1m").arg(feeder->length(), 0, 'f', 2));
    }
    m_points.clear();
    m_drawing = false;
}

void FeederTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentPos = m_view->mapToScene(event->pos());
    if (m_view->isOrthoEnabled() && m_points.size() > 0) {
        QPointF last = m_points.last();
        qreal dx = qAbs(m_currentPos.x() - last.x());
        qreal dy = qAbs(m_currentPos.y() - last.y());
        if (dx > dy) m_currentPos.setY(last.y());
        else m_currentPos.setX(last.x());
    }
    m_lastWorldPos = m_currentPos;
    m_view->viewport()->update();
}

void FeederTool::mouseReleaseEvent(QMouseEvent *) {}

void FeederTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        m_points.clear();
        m_drawing = false;
        emit finished();
        event->accept();
    } else if (event->key() == Qt::Key_U && m_drawing) {
        // 放弃上一点
        if (m_points.size() > 1) m_points.removeLast();
        else { m_points.clear(); m_drawing = false; }
        event->accept();
    } else if (event->key() == Qt::Key_C && m_drawing) {
        // 闭合
        if (m_points.size() >= 3) {
            m_points.append(m_points.first());
            FeederItem *feeder = new FeederItem(m_points, m_feederType);
            feeder->setLayer("馈线");
            m_scene->addItem(feeder);
        }
        m_points.clear();
        m_drawing = false;
        event->accept();
    }
}

void FeederTool::drawOverlay(QPainter *painter)
{
    if (!m_drawing || m_points.isEmpty()) return;

    painter->save();
    painter->setPen(QPen(QColor(0, 200, 255, 180), 2, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);

    // 绘制已确定的线段
    for (int i = 1; i < m_points.size(); i++) {
        painter->drawLine(m_view->mapFromScene(m_points[i-1]), m_view->mapFromScene(m_points[i]));
    }

    // 绘制当前预览线段
    painter->drawLine(m_view->mapFromScene(m_points.last()), m_view->mapFromScene(m_currentPos));

    // 绘制点
    painter->setBrush(QColor(0, 200, 255));
    for (const auto &p : m_points) {
        painter->drawEllipse(m_view->mapFromScene(p), 3, 3);
    }

    painter->restore();
}

void FeederTool::deactivate()
{
    m_points.clear();
    m_drawing = false;
}
