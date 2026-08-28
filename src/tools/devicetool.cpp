#include "devicetool.h"
#include "cadview.h"
#include "cadscene.h"
#include <QMouseEvent>
#include <QPainter>

DeviceTool::DeviceTool(CadView *view, DeviceType type, QObject *parent)
    : Tool(view, parent), m_deviceType(type) {}

void DeviceTool::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    QPointF wp = m_view->mapToScene(event->pos());
    if (m_scene) {
        DeviceItem *dev = new DeviceItem(m_deviceType);
        dev->setPos(wp);
        m_scene->addItem(dev);
        emit statusMessage(QString("已放置: %1").arg(dev->deviceTypeName()));
    }
}

void DeviceTool::mouseMoveEvent(QMouseEvent *event)
{
    m_currentPos = m_view->mapToScene(event->pos());
    m_lastWorldPos = m_currentPos;
}

void DeviceTool::mouseReleaseEvent(QMouseEvent *) {}

void DeviceTool::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) { emit finished(); event->accept(); }
}

void DeviceTool::drawOverlay(QPainter *painter)
{
    // 显示放置预览
    QPoint sp = m_view->mapFromScene(m_currentPos);
    painter->save();
    painter->setPen(QPen(QColor(255, 255, 0), 1, Qt::DashLine));
    painter->setBrush(QColor(255, 255, 0, 30));
    painter->drawEllipse(sp, 12, 12);
    painter->restore();
}
