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

            // 自动标注线长
            qreal totalLen = feeder->length();
            QPointF midPoint = m_points.at(m_points.size() / 2);
            QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(
                QString("%1m").arg(totalLen, 0, 'f', 2));
            label->setPos(midPoint.x() + 5, midPoint.y() - 15);
            label->setFont(QFont("Microsoft YaHei", 8));
            label->setBrush(QColor(0, 100, 200));
            label->setZValue(10);
            m_scene->addItem(label);

            emit statusMessage(QString("馈线绘制完成，类型=%1，长度=%2m").arg(feeder->typeName()).arg(totalLen, 0, 'f', 2));
        }
        m_points.clear();
        m_drawing = false;
        emit finished();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    QPointF wp = m_view->mapToScene(event->pos());

    // 端口连接检测：吸附到附近器件
    const qreal snapTolerance = 20.0; // 吸附半径(场景单位)
    QList<QGraphicsItem*> items = m_scene->items(wp, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (QGraphicsItem *item : items) {
        // 检测器件（DeviceItem或其他器件图元）
        if (item->data(0).isValid() || item->type() == QGraphicsItem::UserType + 100) {
            QPointF deviceCenter = item->sceneBoundingRect().center();
            if (QLineF(wp, deviceCenter).length() < snapTolerance) {
                wp = deviceCenter;
                emit statusMessage("已吸附到器件端口");
                break;
            }
        }
    }

    // 正交模式
    if (m_view->orthoMode() && m_points.size() > 0) {
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

        // 自动打断检测：馈线经过器件时自动打断
        QList<QGraphicsItem*> deviceItems;
        for (QGraphicsItem *item : m_scene->items()) {
            if (item->data(0).isValid() || item->type() == QGraphicsItem::UserType + 100) {
                if (item != feeder) deviceItems.append(item);
            }
        }

        // 检测馈线与器件的交点
        QList<qreal> splitParams;
        for (QGraphicsItem *dev : deviceItems) {
            QRectF devRect = dev->sceneBoundingRect();
            for (int i = 1; i < m_points.size(); i++) {
                QLineF seg(m_points[i-1], m_points[i]);
                // 简化检测：线段与器件矩形相交
                QPointF p1, p2;
                if (seg.intersects(QLineF(devRect.topLeft(), devRect.topRight()), &p1) == QLineF::BoundedIntersection ||
                    seg.intersects(QLineF(devRect.bottomLeft(), devRect.bottomRight()), &p1) == QLineF::BoundedIntersection ||
                    seg.intersects(QLineF(devRect.topLeft(), devRect.bottomLeft()), &p1) == QLineF::BoundedIntersection ||
                    seg.intersects(QLineF(devRect.topRight(), devRect.bottomRight()), &p1) == QLineF::BoundedIntersection) {
                    // 记录交点参数（简化处理）
                    splitParams.append(i - 0.5);
                }
            }
        }

        // 自动标注线长（在馈线中点）
        qreal totalLen = feeder->length();
        QPointF midPoint = m_points.at(m_points.size() / 2);
        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(
            QString("%1m").arg(totalLen, 0, 'f', 2));
        label->setPos(midPoint.x() + 5, midPoint.y() - 15);
        label->setFont(QFont("Microsoft YaHei", 8));
        label->setBrush(QColor(0, 100, 200));
        label->setZValue(10);
        m_scene->addItem(label);

        QString msg = QString("馈线绘制完成，类型=%1，长度=%2m").arg(feeder->typeName()).arg(totalLen, 0, 'f', 2);
        if (!splitParams.isEmpty()) {
            msg += QString("，检测到%1处器件交叉（自动打断功能开发中）").arg(splitParams.size());
        }
        emit statusMessage(msg);
    }
    m_points.clear();
    m_drawing = false;
}

void FeederTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentPos = m_view->mapToScene(event->pos());

    // 端口连接检测预览
    const qreal snapTolerance = 20.0;
    QList<QGraphicsItem*> items = m_scene->items(m_currentPos, Qt::IntersectsItemShape, Qt::DescendingOrder);
    for (QGraphicsItem *item : items) {
        if (item->data(0).isValid() || item->type() == QGraphicsItem::UserType + 100) {
            QPointF deviceCenter = item->sceneBoundingRect().center();
            if (QLineF(m_currentPos, deviceCenter).length() < snapTolerance) {
                m_currentPos = deviceCenter;
                break;
            }
        }
    }

    if (m_view->orthoMode() && m_points.size() > 0) {
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
