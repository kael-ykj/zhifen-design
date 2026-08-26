#include "canvaswidget.h"
#include <QPaintEvent>
#include <QKeyEvent>
#include <algorithm>
#include <cmath>

CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

CanvasWidget::~CanvasWidget() {}

void CanvasWidget::setProject(zf::Project* project)
{
    m_project = project;
    update();
}

void CanvasWidget::setModeManager(zf::ModeManager* mgr)
{
    m_modeMgr = mgr;
    update();
}

void CanvasWidget::setCurrentTool(const QString& tool)
{
    if (m_currentTool != tool) {
        m_drawingWall = false;
        m_drawingCable = false;
        m_cableStartDeviceId.clear();
    }
    m_currentTool = tool;
    if (tool == "select") setCursor(Qt::ArrowCursor);
    else if (tool == "place") setCursor(Qt::CrossCursor);
    else if (tool == "wall") setCursor(Qt::CrossCursor);
    else if (tool == "cable") setCursor(Qt::CrossCursor);
    else setCursor(Qt::ArrowCursor);
}

void CanvasWidget::setPlaceModel(const QString& modelId)
{
    m_placeModelId = modelId;
    if (!modelId.isEmpty()) {
        setCurrentTool("place");
        emit statusMessage("准备放置: " + modelId);
    }
}


void CanvasWidget::setCurrentFloorIndex(int index)
{
    if (m_project && index >= 0 && index < (int)m_project->floors.size()) {
        m_currentFloorIndex = index;
        m_selectedDeviceId.clear();
        m_drawingWall = false;
        m_drawingCable = false;
        m_cableStartDeviceId.clear();
        update();
    }
}
void CanvasWidget::setHeatmap(const zf::HeatmapData& heatmap)
{
    m_heatmap = heatmap;
    m_hasHeatmap = true;
    update();
}

void CanvasWidget::clearHeatmap()
{
    m_hasHeatmap = false;
    update();
}

void CanvasWidget::deleteSelectedDevice()
{
    if (m_selectedDeviceId.isEmpty() || !m_project || m_project->floors.empty()) return;
    emit projectAboutToChange();
    QString deletedId = m_selectedDeviceId;
    auto& floor = m_project->floors[m_currentFloorIndex];
    floor.devices.erase(std::remove_if(floor.devices.begin(), floor.devices.end(),
        [&](const zf::DeviceInstance& d) { return d.instanceId == deletedId.toStdString(); }),
        floor.devices.end());
    for (auto& dev : floor.devices) {
        dev.connections.erase(std::remove_if(dev.connections.begin(), dev.connections.end(),
            [&](const zf::DeviceInstance::Connection& c) { return c.targetInstanceId == deletedId.toStdString(); }),
            dev.connections.end());
    }
    m_selectedDeviceId.clear();
    emit deviceDeleted(deletedId);
    emit projectChanged("删除器件");
    update();
}

void CanvasWidget::refresh()
{
    update();
}

QPixmap CanvasWidget::exportToImage(int width, int height)
{
    QPixmap pixmap(width, height);
    pixmap.fill(QColor(245, 245, 245));
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    // 计算缩放比例，使内容居中
    double scaleX = (double)width / this->width();
    double scaleY = (double)height / this->height();
    double scale = std::min(scaleX, scaleY);
    painter.translate((width - this->width() * scale) / 2, (height - this->height() * scale) / 2);
    painter.scale(scale, scale);
    // 渲染画布
    this->render(&painter);
    painter.end();
    return pixmap;
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(245, 245, 245));
    if (!m_project || m_project->floors.empty()) return;

    drawGrid(painter);
    if (m_hasHeatmap) drawHeatmap(painter);
    drawWalls(painter);
    drawCables(painter);
    drawDevices(painter);
    drawSelectedHighlight(painter);

    // 墙体绘制预览
    if (m_drawingWall) {
        QPen previewPen(QColor(255, 100, 0));
        previewPen.setWidth(2);
        previewPen.setStyle(Qt::DashLine);
        painter.setPen(previewPen);
        painter.setBrush(Qt::NoBrush);
        QPointF start = worldToScreen(m_wallStartPoint);
        QPointF end = worldToScreen(m_wallPreviewPoint);
        painter.drawLine(start, end);
        painter.setBrush(QColor(255, 100, 0));
        painter.drawEllipse(start, 4, 4);
    }

    // 线缆连接预览
    if (m_drawingCable && !m_cableStartDeviceId.isEmpty()) {
        // 找到起点器件位置
        QPointF startPos;
        bool found = false;
        const auto& floor = m_project->floors[m_currentFloorIndex];
        for (const auto& dev : floor.devices) {
            if (dev.instanceId == m_cableStartDeviceId.toStdString()) {
                startPos = worldToScreen(QPointF(dev.position.x, dev.position.y));
                found = true;
                break;
            }
        }
        if (found) {
            QPen cablePen(QColor(0, 160, 0));
            cablePen.setWidth(3);
            cablePen.setStyle(Qt::DashLine);
            painter.setPen(cablePen);
            painter.setBrush(Qt::NoBrush);
            QPointF end = worldToScreen(m_cablePreviewPoint);
            painter.drawLine(startPos, end);
            painter.setBrush(QColor(0, 160, 0));
            painter.drawEllipse(startPos, 5, 5);
        }
    }

    // 模式指示
    if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == zf::WorkMode::SKETCH_MODE) {
        painter.setPen(QColor(200, 100, 0));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(10, 25, "草图模式 - 重型计算受限");
    } else {
        painter.setPen(QColor(0, 120, 0));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(10, 25, "正式工程模式");
    }

    // 热力图图例
    if (m_hasHeatmap) {
        int legendX = width() - 180;
        int legendY = 30;
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 9));
        painter.drawText(legendX, legendY - 5, "信号强度 (RSRP dBm)");
        for (int i = 0; i < 20; i++) {
            double rsrp = -140.0 + i * 4.0;
            painter.fillRect(legendX + i * 8, legendY, 8, 15, rsrpToColor(rsrp));
        }
        painter.setPen(Qt::black);
        painter.drawText(legendX, legendY + 28, "-140");
        painter.drawText(legendX + 130, legendY + 28, "-60");
    }

    painter.setPen(Qt::gray);
    painter.setFont(QFont("Arial", 9));
    painter.drawText(10, height() - 10, QString("缩放: %1%").arg(m_zoom * 100, 0, 'f', 0));
}

void CanvasWidget::drawGrid(QPainter& painter)
{
    painter.setPen(QColor(220, 220, 220));
    double gridSize = 50.0 * m_zoom;
    if (gridSize < 5) gridSize = 5;
    QPointF origin = worldToScreen(QPointF(0, 0));
    double startX = fmod(origin.x(), gridSize);
    double startY = fmod(origin.y(), gridSize);
    for (double x = startX; x < width(); x += gridSize)
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    for (double y = startY; y < height(); y += gridSize)
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    painter.setPen(QColor(180, 180, 180));
    painter.drawLine(origin, QPointF(width(), origin.y()));
    painter.drawLine(origin, QPointF(origin.x(), height()));
}

QColor CanvasWidget::rsrpToColor(double rsrp) const
{
    // -140 (红) -> -100 (黄) -> -60 (绿)
    double t;
    if (rsrp <= -140) return QColor(180, 30, 30, 120);
    if (rsrp >= -60) return QColor(30, 180, 30, 120);
    if (rsrp < -100) {
        t = (rsrp + 140) / 40.0;
        return QColor(180, (int)(30 + t * 180), 30, 120);
    } else {
        t = (rsrp + 100) / 40.0;
        return QColor((int)(180 - t * 150), 180, 30, 120);
    }
}

void CanvasWidget::drawHeatmap(QPainter& painter)
{
    if (m_heatmap.points.empty()) return;
    double cellSize = m_heatmap.gridResolution_m * m_zoom;
    if (cellSize < 2) cellSize = 2;
    for (const auto& pt : m_heatmap.points) {
        QPointF screen = worldToScreen(QPointF(pt.position.x, pt.position.y));
        painter.fillRect(QRectF(screen.x() - cellSize/2, screen.y() - cellSize/2,
                                 cellSize, cellSize), rsrpToColor(pt.rsrp_dBm));
    }
}

void CanvasWidget::drawWalls(QPainter& painter)
{
    if (!m_project) return;
    const auto& floor = m_project->floors[m_currentFloorIndex];
    QPen wallPen(QColor(80, 80, 80));
    wallPen.setWidth(3);
    painter.setPen(wallPen);
    for (const auto& wall : floor.walls) {
        for (size_t i = 0; i + 1 < wall.points.size(); i++) {
            QPointF p1 = worldToScreen(QPointF(wall.points[i].x, wall.points[i].y));
            QPointF p2 = worldToScreen(QPointF(wall.points[i+1].x, wall.points[i+1].y));
            painter.drawLine(p1, p2);
        }
    }
}

void CanvasWidget::drawCables(QPainter& painter)
{
    if (!m_project) return;
    const auto& floor = m_project->floors[m_currentFloorIndex];
    QPen cablePen(QColor(0, 150, 0));
    cablePen.setWidth(2);
    cablePen.setStyle(Qt::DashLine);
    painter.setPen(cablePen);
    for (const auto& dev : floor.devices) {
        for (const auto& conn : dev.connections) {
            const zf::DeviceInstance* target = nullptr;
            for (const auto& d : floor.devices) {
                if (d.instanceId == conn.targetInstanceId) { target = &d; break; }
            }
            if (target) {
                QPointF p1 = worldToScreen(QPointF(dev.position.x, dev.position.y));
                QPointF p2 = worldToScreen(QPointF(target->position.x, target->position.y));
                painter.drawLine(p1, p2);
            }
        }
    }
}

void CanvasWidget::drawDevices(QPainter& painter)
{
    if (!m_project) return;
    const auto& floor = m_project->floors[m_currentFloorIndex];
    for (const auto& dev : floor.devices) {
        QPointF pos = worldToScreen(QPointF(dev.position.x, dev.position.y));
        double r = 12 * m_zoom;
        if (r < 4) r = 4;
        QColor color;
        QString label;
        auto it = std::find_if(m_project->deviceLibrary.begin(), m_project->deviceLibrary.end(),
            [&](const zf::DeviceModel& m) { return m.modelId == dev.modelId; });
        if (it != m_project->deviceLibrary.end()) {
            switch (it->category) {
                case zf::DeviceCategory::SIGNAL_SOURCE: color = QColor(200, 50, 50); label = "信源"; break;
                case zf::DeviceCategory::ANTENNA: color = QColor(50, 100, 200); label = "天线"; break;
                case zf::DeviceCategory::SPLITTER: color = QColor(200, 150, 50); label = "功分"; break;
                case zf::DeviceCategory::COUPLER: color = QColor(150, 50, 200); label = "耦合"; break;
                case zf::DeviceCategory::COMBINER: color = QColor(50, 180, 180); label = "合路"; break;
                default: color = QColor(100, 100, 100); label = "器件"; break;
            }
        } else {
            color = QColor(100, 100, 100); label = "?";
        }
        painter.setBrush(color);
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(pos, r, r);
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(pos + QPointF(r + 2, 4), QString::fromStdString(dev.instanceId));
        if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == zf::WorkMode::FORMAL_MODE) {
            if (!dev.outputPower_dBm.empty()) {
                auto pit = dev.outputPower_dBm.begin();
                painter.setPen(QColor(0, 100, 0));
                painter.setFont(QFont("Arial", 7));
                painter.drawText(pos + QPointF(-r, -r - 2),
                    QString::number(pit->second, 'f', 1) + "dBm");
            }
        }
    }
}

void CanvasWidget::drawSelectedHighlight(QPainter& painter)
{
    if (m_selectedDeviceId.isEmpty() || !m_project) return;
    const auto& floor = m_project->floors[m_currentFloorIndex];
    for (const auto& dev : floor.devices) {
        if (dev.instanceId == m_selectedDeviceId.toStdString()) {
            QPointF pos = worldToScreen(QPointF(dev.position.x, dev.position.y));
            double r = 18 * m_zoom;
            painter.setBrush(Qt::NoBrush);
            QPen pen(QColor(255, 150, 0));
            pen.setWidth(2);
            pen.setStyle(Qt::DashLine);
            painter.setPen(pen);
            painter.drawEllipse(pos, r, r);
            break;
        }
    }
}

QPointF CanvasWidget::worldToScreen(const QPointF& world) const
{
    return QPointF(
        world.x() * m_zoom + m_panOffset.x() + width() / 2.0,
        world.y() * m_zoom + m_panOffset.y() + height() / 2.0
    );
}

QPointF CanvasWidget::screenToWorld(const QPointF& screen) const
{
    return QPointF(
        (screen.x() - m_panOffset.x() - width() / 2.0) / m_zoom,
        (screen.y() - m_panOffset.y() - height() / 2.0) / m_zoom
    );
}

QString CanvasWidget::findDeviceAt(const QPointF& worldPos)
{
    if (!m_project || m_project->floors.empty()) return "";
    const auto& floor = m_project->floors[m_currentFloorIndex];
    double threshold = 15.0 / m_zoom;
    for (const auto& dev : floor.devices) {
        double dx = dev.position.x - worldPos.x();
        double dy = dev.position.y - worldPos.y();
        if (std::sqrt(dx*dx + dy*dy) < threshold)
            return QString::fromStdString(dev.instanceId);
    }
    return "";
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->localPos();
    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    QPointF worldPos = screenToWorld(event->localPos());
    if (m_currentTool == "select" && event->button() == Qt::LeftButton) {
        QString devId = findDeviceAt(worldPos);
        m_selectedDeviceId = devId;
        if (!devId.isEmpty()) {
            emit deviceSelected(devId);
            emit projectAboutToChange();
            m_dragging = true;
        }
        update();
    }
    else if (m_currentTool == "place" && event->button() == Qt::LeftButton) {
        if (!m_project || m_placeModelId.isEmpty()) {
            emit statusMessage("请先从器件库选择要放置的器件");
            return;
        }
        zf::DeviceInstance dev;
        emit projectAboutToChange();
        dev.instanceId = "DEV_" + std::to_string(m_project->floors[m_currentFloorIndex].devices.size() + 1);
        dev.modelId = m_placeModelId.toStdString();
        dev.position = {worldPos.x(), worldPos.y()};
        m_project->floors[m_currentFloorIndex].devices.push_back(dev);
        emit statusMessage("已放置: " + m_placeModelId + " at (" +
            QString::number(worldPos.x(), 'f', 0) + ", " +
            QString::number(worldPos.y(), 'f', 0) + ")");
        emit projectChanged("放置器件");
        update();
    }
    else if (m_currentTool == "wall" && event->button() == Qt::LeftButton) {
        if (!m_project) return;
        if (!m_drawingWall) {
            // 开始绘制墙体
            m_wallStartPoint = worldPos;
            m_wallPreviewPoint = worldPos;
            m_drawingWall = true;
            emit statusMessage("墙体绘制中: 点击设置终点，右键/Esc结束");
        } else {
            // 完成一段墙体
            zf::Wall wall;
            emit projectAboutToChange();
            wall.wallId = "WALL_" + std::to_string(m_project->floors[m_currentFloorIndex].walls.size() + 1);
            wall.points.push_back({m_wallStartPoint.x(), m_wallStartPoint.y()});
            wall.points.push_back({worldPos.x(), worldPos.y()});
            wall.attenuation_dB = 10.0; // 默认墙体损耗
            wall.thickness_mm = 240.0;
            m_project->floors[m_currentFloorIndex].walls.push_back(wall);
            emit projectChanged("绘制墙体");
            double len = std::sqrt(std::pow(worldPos.x() - m_wallStartPoint.x(), 2) +
                                    std::pow(worldPos.y() - m_wallStartPoint.y(), 2));
            emit statusMessage(QString("已添加墙体段 (长度: %1m)，继续点击添加下一段，右键结束").arg(len, 0, 'f', 1));
            // 连续绘制：当前终点作为下一段起点
            m_wallStartPoint = worldPos;
            m_wallPreviewPoint = worldPos;
        }
        update();
    }
    else if (m_currentTool == "wall" && event->button() == Qt::RightButton) {
        if (m_drawingWall) {
            m_drawingWall = false;
            emit statusMessage("墙体绘制结束");
            update();
        }
    }
    else if (m_currentTool == "cable" && event->button() == Qt::LeftButton) {
        if (!m_project) return;
        QString devId = findDeviceAt(worldPos);
        if (devId.isEmpty()) {
            emit statusMessage("请点击器件来创建连接");
            return;
        }
        if (!m_drawingCable) {
            // 第一次点击：设置起点
            m_drawingCable = true;
            m_cableStartDeviceId = devId;
            m_cablePreviewPoint = worldPos;
            emit statusMessage("线缆连接: 已选起点 " + devId + "，点击目标器件完成连接");
        } else {
            // 第二次点击：创建连接
            if (devId == m_cableStartDeviceId) {
                emit statusMessage("不能连接到自身，请选择其他器件");
                return;
            }
            auto& floor = m_project->floors[m_currentFloorIndex];
            // 在起点器件的connections中添加指向目标的连接
            for (auto& dev : floor.devices) {
                if (dev.instanceId == m_cableStartDeviceId.toStdString()) {
                    // 检查是否已存在连接
                    bool exists = false;
                    for (const auto& conn : dev.connections) {
                        if (conn.targetInstanceId == devId.toStdString()) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        zf::DeviceInstance::Connection conn;
                        emit projectAboutToChange();
                        conn.targetInstanceId = devId.toStdString();
                        conn.fromPortId = "out";
                        conn.toPortId = "in";
                        dev.connections.push_back(conn);
                        emit projectChanged("线缆连接");
                    }
                    break;
                }
            }
            emit statusMessage("已创建连接: " + m_cableStartDeviceId + " → " + devId + "，可继续点击下一个目标，右键结束");
            // 连续连接：当前目标作为下一个起点
            m_cableStartDeviceId = devId;
            m_cablePreviewPoint = worldPos;
        }
        update();
    }
    else if (m_currentTool == "cable" && event->button() == Qt::RightButton) {
        if (m_drawingCable) {
            m_drawingCable = false;
            m_cableStartDeviceId.clear();
            emit statusMessage("线缆连接结束");
            update();
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF delta = event->localPos() - m_lastMousePos;
    if (m_panning) {
        m_panOffset += delta;
        update();
    }
    else if (m_dragging && !m_selectedDeviceId.isEmpty() && m_project) {
        QPointF worldPos = screenToWorld(event->localPos());
        auto& floor = m_project->floors[m_currentFloorIndex];
        for (auto& dev : floor.devices) {
            if (dev.instanceId == m_selectedDeviceId.toStdString()) {
                dev.position = {worldPos.x(), worldPos.y()};
                break;
            }
        }
        update();
    }
    else if (m_drawingWall) {
        m_wallPreviewPoint = screenToWorld(event->localPos());
        update();
    }
    else if (m_drawingCable) {
        m_cablePreviewPoint = screenToWorld(event->localPos());
        update();
    }
    m_lastMousePos = event->localPos();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        setCurrentTool(m_currentTool);
    }
    if (event->button() == Qt::LeftButton) {
        if (m_dragging && !m_selectedDeviceId.isEmpty()) {
            emit statusMessage("已移动器件: " + m_selectedDeviceId);
            emit projectChanged("移动器件");
        }
        m_dragging = false;
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
    double newZoom = m_zoom * factor;
    if (newZoom < 0.1) newZoom = 0.1;
    if (newZoom > 20) newZoom = 20;
    QPointF mousePos = event->pos();
    QPointF worldBefore = screenToWorld(mousePos);
    m_zoom = newZoom;
    QPointF worldAfter = screenToWorld(mousePos);
    m_panOffset += (worldAfter - worldBefore) * m_zoom;
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (!m_selectedDeviceId.isEmpty()) {
            deleteSelectedDevice();
        }
    } else if (event->key() == Qt::Key_Escape) {
        if (m_drawingWall) {
            m_drawingWall = false;
            emit statusMessage("墙体绘制已取消");
            update();
        } else if (m_drawingCable) {
            m_drawingCable = false;
            m_cableStartDeviceId.clear();
            emit statusMessage("线缆连接已取消");
            update();
        } else {
            m_selectedDeviceId.clear();
            m_placeModelId.clear();
            setCurrentTool("select");
            update();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}
