#include "canvaswidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>
#include <QFontMetrics>

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
    m_selectedDeviceId.clear();
    m_activeFloorIndex = 0;
    if (m_project && m_project->layers.empty()) {
        m_project->initDefaultLayers();
    }
    update();
}

void CanvasWidget::setModeManager(zf::ModeManager* mgr)
{
    m_modeMgr = mgr;
    update();
}

void CanvasWidget::setCurrentTool(const QString& tool)
{
    m_currentTool = tool;
    if (tool == "select") {
        setCursor(Qt::ArrowCursor);
    } else if (tool == "place" || tool == "wall" || tool == "cable") {
        setCursor(Qt::CrossCursor);
    } else if (tool == "pan") {
        setCursor(Qt::OpenHandCursor);
    }
    update();
}

void CanvasWidget::setPlaceModel(const QString& modelId)
{
    m_placeModelId = modelId;
}

void CanvasWidget::setActiveFloorIndex(int index)
{
    if (m_project && index >= 0 && index < (int)m_project->floors.size()) {
        m_activeFloorIndex = index;
        emit activeFloorChanged(index);
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
    for (auto& floor : m_project->floors) {
        floor.devices.erase(std::remove_if(floor.devices.begin(), floor.devices.end(),
            [&](const zf::DeviceInstance& d) { return d.instanceId == deletedId.toStdString(); }),
            floor.devices.end());
        for (auto& dev : floor.devices) {
            dev.connections.erase(std::remove_if(dev.connections.begin(), dev.connections.end(),
                [&](const zf::DeviceInstance::Connection& c) { return c.targetInstanceId == deletedId.toStdString(); }),
                dev.connections.end());
        }
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
    double scaleX = (double)width / this->width();
    double scaleY = (double)height / this->height();
    double scale = std::min(scaleX, scaleY);
    painter.translate((width - this->width() * scale) / 2, (height - this->height() * scale) / 2);
    painter.scale(scale, scale);
    this->render(&painter);
    painter.end();
    return pixmap;
}

QPointF CanvasWidget::worldToScreen(const QPointF& world) const
{
    return QPointF(world.x() * m_zoom + m_panOffset.x(),
                   height() - (world.y() * m_zoom + m_panOffset.y()));
}

QPointF CanvasWidget::screenToWorld(const QPointF& screen) const
{
    return QPointF((screen.x() - m_panOffset.x()) / m_zoom,
                   (height() - screen.y() - m_panOffset.y()) / m_zoom);
}

QPointF CanvasWidget::snapPoint(const QPointF& worldPos) const
{
    if (!m_snapEnabled) return worldPos;
    double grid = 100.0;
    return QPointF(round(worldPos.x() / grid) * grid,
                   round(worldPos.y() / grid) * grid);
}

int CanvasWidget::findFloorAt(const QPointF& worldPos) const
{
    if (!m_project) return -1;
    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        QRectF bounds = getFloorBounds(i);
        if (bounds.contains(worldPos)) return i;
    }
    return -1;
}

QRectF CanvasWidget::getFloorBounds(int floorIndex) const
{
    if (!m_project || floorIndex < 0 || floorIndex >= (int)m_project->floors.size())
        return QRectF();
    const auto& floor = m_project->floors[floorIndex];
    double ox = floor.origin.x;
    double oy = floor.origin.y;
    return QRectF(ox, oy, 30000, 20000);
}

QRectF CanvasWidget::getAllContentBounds() const
{
    if (!m_project || m_project->floors.empty()) return QRectF(0, 0, 30000, 20000);
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        QRectF b = getFloorBounds(i);
        minX = std::min(minX, b.left());
        minY = std::min(minY, b.bottom());
        maxX = std::max(maxX, b.right());
        maxY = std::max(maxY, b.top());
    }
    double sysX = maxX + 10000;
    maxX = sysX + 40000;
    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void CanvasWidget::goToFloor(int floorIndex)
{
    if (!m_project || floorIndex < 0 || floorIndex >= (int)m_project->floors.size()) return;
    QRectF bounds = getFloorBounds(floorIndex);
    double centerX = bounds.center().x();
    double centerY = bounds.center().y();
    m_zoom = std::min((double)width() / bounds.width(), (double)height() / bounds.height()) * 0.9;
    m_panOffset = QPointF(width() / 2 - centerX * m_zoom,
                          height() / 2 - centerY * m_zoom);
    m_activeFloorIndex = floorIndex;
    emit activeFloorChanged(floorIndex);
    update();
}

void CanvasWidget::goToSystemDiagram()
{
    if (!m_project || m_project->floors.empty()) return;
    QRectF allBounds = getAllContentBounds();
    double sysX = allBounds.right() - 40000;
    double centerX = sysX + 20000;
    double centerY = allBounds.center().y();
    m_zoom = std::min((double)width() / 40000.0, (double)height() / allBounds.height()) * 0.9;
    m_panOffset = QPointF(width() / 2 - centerX * m_zoom,
                          height() / 2 - centerY * m_zoom);
    update();
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(245, 245, 245));
    if (!m_project || m_project->floors.empty()) return;

    drawGrid(painter);
    drawFloorSeparators(painter);

    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        const auto& floor = m_project->floors[i];
        if (m_project->isLayerVisible(zf::LayerType::WALL))
            drawWallsForFloor(painter, floor);
        if (m_project->isLayerVisible(zf::LayerType::CABLE))
            drawCablesForFloor(painter, floor);
        if (m_project->isLayerVisible(zf::LayerType::DEVICE))
            drawDevicesForFloor(painter, floor);
    }

    if (m_hasHeatmap && m_project->isLayerVisible(zf::LayerType::HEATMAP))
        drawHeatmap(painter);

    if (m_project->isLayerVisible(zf::LayerType::SYSTEM_DIAGRAM))
        drawSystemDiagramArea(painter);

    drawFloorLabels(painter);
    drawSelectedHighlight(painter);

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

    if (m_drawingCable && !m_cableStartDeviceId.isEmpty()) {
        QPointF startPos;
        bool found = false;
        for (const auto& floor : m_project->floors) {
            for (const auto& dev : floor.devices) {
                if (dev.instanceId == m_cableStartDeviceId.toStdString()) {
                    startPos = worldToScreen(QPointF(dev.position.x + floor.origin.x, dev.position.y + floor.origin.y));
                    found = true;
                    break;
                }
            }
            if (found) break;
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

    if (m_selectingPrintWindow) {
        drawPrintWindowPreview(painter);
    }

    if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == zf::WorkMode::SKETCH_MODE) {
        painter.setPen(QColor(200, 100, 0));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(10, 25, "草图模式 - 重型计算受限");
    } else {
        painter.setPen(QColor(0, 120, 0));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(10, 25, "正式工程模式");
    }

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
    painter.drawText(10, height() - 10, QString("缩放: %1%  楼层: %2")
        .arg(m_zoom * 100, 0, 'f', 0)
        .arg(m_activeFloorIndex + 1));
}

void CanvasWidget::drawGrid(QPainter& painter)
{
    if (!m_project || !m_project->isLayerVisible(zf::LayerType::AUXILIARY)) return;
    painter.setPen(QColor(220, 220, 220));
    double gridSize = 500.0 * m_zoom;
    if (gridSize < 5) gridSize = 5;
    QPointF origin = worldToScreen(QPointF(0, 0));
    double startX = fmod(origin.x(), gridSize);
    double startY = fmod(origin.y(), gridSize);
    for (double x = startX; x < width(); x += gridSize)
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    for (double y = startY; y < height(); y += gridSize)
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
}

void CanvasWidget::drawFloorSeparators(QPainter& painter)
{
    if (!m_project || !m_project->isLayerVisible(zf::LayerType::AUXILIARY)) return;
    QPen sepPen(QColor(200, 100, 100));
    sepPen.setStyle(Qt::DashLine);
    sepPen.setWidth(1);
    painter.setPen(sepPen);
    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        QRectF bounds = getFloorBounds(i);
        QPointF tl = worldToScreen(QPointF(bounds.left(), bounds.top()));
        QPointF br = worldToScreen(QPointF(bounds.right(), bounds.bottom()));
        painter.drawRect(QRectF(tl, br));
    }
    if (!m_project->floors.empty()) {
        QRectF allBounds = getAllContentBounds();
        double sysX = allBounds.right() - 40000;
        QPointF tl = worldToScreen(QPointF(sysX, allBounds.top()));
        QPointF br = worldToScreen(QPointF(allBounds.right(), allBounds.bottom()));
        painter.setPen(QPen(QColor(100, 100, 200), 1, Qt::DashLine));
        painter.drawRect(QRectF(tl, br));
    }
}

void CanvasWidget::drawFloorLabels(QPainter& painter)
{
    if (!m_project || !m_project->isLayerVisible(zf::LayerType::ANNOTATION)) return;
    QFont labelFont("Arial", 14, QFont::Bold);
    painter.setFont(labelFont);
    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        const auto& floor = m_project->floors[i];
        QRectF bounds = getFloorBounds(i);
        QPointF labelPos = worldToScreen(QPointF(bounds.left() + 500, bounds.top() - 500));
        painter.setPen(i == m_activeFloorIndex ? QColor(200, 50, 50) : QColor(80, 80, 80));
        QString label = QString::fromStdString(floor.floorName.empty() ?
            ("楼层 " + std::to_string(i + 1)) : floor.floorName);
        painter.drawText(labelPos, label + " (双击定位)");
    }
    if (!m_project->floors.empty()) {
        QRectF allBounds = getAllContentBounds();
        double sysX = allBounds.right() - 40000;
        QPointF labelPos = worldToScreen(QPointF(sysX + 500, allBounds.top() - 500));
        painter.setPen(QColor(50, 50, 200));
        painter.drawText(labelPos, "系统图 (双击定位)");
    }
}

void CanvasWidget::drawSystemDiagramArea(QPainter& painter)
{
    if (!m_project || m_project->systemDiagrams.empty()) return;
    QRectF allBounds = getAllContentBounds();
    double sysX = allBounds.right() - 40000;
    double sysY = allBounds.bottom() + 1000;

    painter.setPen(QColor(80, 80, 180));
    painter.setFont(QFont("Arial", 11));
    QPointF pos = worldToScreen(QPointF(sysX + 1000, sysY + 1000));
    painter.drawText(pos, QString("系统图区域 (%1 个)").arg(m_project->systemDiagrams.size()));

    for (size_t di = 0; di < m_project->systemDiagrams.size(); di++) {
        double yOffset = di * 3000;
        QPointF srcScreen = worldToScreen(QPointF(sysX + 2000, sysY + 2000 + yOffset));
        painter.setBrush(QColor(255, 200, 100));
        painter.setPen(QColor(0, 0, 0));
        painter.drawRect(QRectF(srcScreen.x() - 20, srcScreen.y() - 15, 40, 30));
        painter.drawText(srcScreen + QPointF(-15, 5), "信源");
    }
}

QColor CanvasWidget::rsrpToColor(double rsrp) const
{
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
    double cellSize = m_heatmap.gridResolution_m * 1000 * m_zoom;
    if (cellSize < 2) cellSize = 2;
    for (const auto& pt : m_heatmap.points) {
        QPointF screen = worldToScreen(QPointF(pt.position.x * 1000, pt.position.y * 1000));
        painter.fillRect(QRectF(screen.x() - cellSize/2, screen.y() - cellSize/2,
                                 cellSize, cellSize), rsrpToColor(pt.rsrp_dBm));
    }
}

void CanvasWidget::drawWallsForFloor(QPainter& painter, const zf::Floor& floor)
{
    double ox = floor.origin.x;
    double oy = floor.origin.y;
    QPen wallPen(QColor(80, 80, 80));
    wallPen.setWidth(3);
    painter.setPen(wallPen);
    for (const auto& wall : floor.walls) {
        for (size_t i = 0; i + 1 < wall.points.size(); i++) {
            QPointF p1 = worldToScreen(QPointF(wall.points[i].x + ox, wall.points[i].y + oy));
            QPointF p2 = worldToScreen(QPointF(wall.points[i+1].x + ox, wall.points[i+1].y + oy));
            painter.drawLine(p1, p2);
        }
    }
}

void CanvasWidget::drawCablesForFloor(QPainter& painter, const zf::Floor& floor)
{
    double ox = floor.origin.x;
    double oy = floor.origin.y;
    QPen cablePen(QColor(0, 150, 0));
    cablePen.setWidth(2);
    painter.setPen(cablePen);
    for (const auto& dev : floor.devices) {
        for (const auto& conn : dev.connections) {
            const zf::DeviceInstance* target = nullptr;
            for (const auto& d : floor.devices) {
                if (d.instanceId == conn.targetInstanceId) { target = &d; break; }
            }
            if (target) {
                QPointF p1 = worldToScreen(QPointF(dev.position.x + ox, dev.position.y + oy));
                QPointF p2 = worldToScreen(QPointF(target->position.x + ox, target->position.y + oy));
                painter.drawLine(p1, p2);
            }
        }
    }
    QPen feederPen(QColor(0, 150, 150));
    feederPen.setWidth(2);
    painter.setPen(feederPen);
    for (const auto& cable : floor.cables) {
        if (cable.routePoints.size() < 2) continue;
        QPointF p1 = worldToScreen(QPointF(cable.routePoints[0].x + ox, cable.routePoints[0].y + oy));
        QPointF p2 = worldToScreen(QPointF(cable.routePoints.back().x + ox, cable.routePoints.back().y + oy));
        painter.drawLine(p1, p2);
        if (m_project->isLayerVisible(zf::LayerType::ANNOTATION) && cable.length_m > 0) {
            QPointF mid = (p1 + p2) / 2;
            painter.setPen(QColor(0, 100, 100));
            painter.setFont(QFont("Arial", 8));
            painter.drawText(mid + QPointF(2, -2), QString("%1m").arg(cable.length_m, 0, 'f', 1));
            painter.setPen(feederPen);
        }
    }
}

void CanvasWidget::drawDevicesForFloor(QPainter& painter, const zf::Floor& floor)
{
    double ox = floor.origin.x;
    double oy = floor.origin.y;
    for (const auto& dev : floor.devices) {
        QPointF pos = worldToScreen(QPointF(dev.position.x + ox, dev.position.y + oy));
        QString modelId = QString::fromStdString(dev.modelId);
        QColor color = QColor(0, 180, 0);
        double size = 12;

        if (modelId.contains("ANT", Qt::CaseInsensitive) || modelId.contains("antenna", Qt::CaseInsensitive)) {
            color = QColor(0, 100, 255);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawEllipse(pos, size, size);
            painter.drawLine(pos - QPointF(size, 0), pos + QPointF(size, 0));
            painter.drawLine(pos - QPointF(0, size), pos + QPointF(0, size));
        } else if (modelId.contains("PS", Qt::CaseInsensitive) || modelId.contains("power", Qt::CaseInsensitive)) {
            color = QColor(255, 150, 0);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(QRectF(pos.x() - size, pos.y() - size*0.7, size*2, size*1.4));
        } else if (modelId.contains("T", Qt::CaseInsensitive) || modelId.contains("coupler", Qt::CaseInsensitive)) {
            color = QColor(200, 50, 200);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawRect(QRectF(pos.x() - size*0.7, pos.y() - size, size*1.4, size*2));
        } else if (modelId.contains("CB", Qt::CaseInsensitive) || modelId.contains("combiner", Qt::CaseInsensitive)) {
            color = QColor(255, 200, 0);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawEllipse(pos, size*0.8, size*0.8);
        } else if (modelId.contains("SRC", Qt::CaseInsensitive) || modelId.contains("source", Qt::CaseInsensitive)) {
            color = QColor(255, 80, 80);
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 2));
            painter.drawRect(QRectF(pos.x() - size*1.2, pos.y() - size, size*2.4, size*2));
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.drawText(pos - QPointF(8, 3), "信源");
        } else {
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 1));
            painter.drawEllipse(pos, size*0.7, size*0.7);
        }

        if (m_project->isLayerVisible(zf::LayerType::ANNOTATION) && !dev.instanceId.empty()) {
            painter.setPen(QColor(0, 0, 0));
            painter.setFont(QFont("Arial", 7));
            painter.drawText(pos + QPointF(size + 2, 4), QString::fromStdString(dev.instanceId));
        }
    }
}

void CanvasWidget::drawSelectedHighlight(QPainter& painter)
{
    if (m_selectedDeviceId.isEmpty() || !m_project) return;
    for (const auto& floor : m_project->floors) {
        double ox = floor.origin.x;
        double oy = floor.origin.y;
        for (const auto& dev : floor.devices) {
            if (dev.instanceId == m_selectedDeviceId.toStdString()) {
                QPointF pos = worldToScreen(QPointF(dev.position.x + ox, dev.position.y + oy));
                QPen highlightPen(QColor(255, 0, 0));
                highlightPen.setWidth(2);
                highlightPen.setStyle(Qt::DashLine);
                painter.setPen(highlightPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(pos, 20, 20);
                return;
            }
        }
    }
}

void CanvasWidget::drawPrintWindowPreview(QPainter& painter)
{
    if (m_printWindowRect.isNull()) return;
    QPointF tl = worldToScreen(QPointF(m_printWindowRect.left(), m_printWindowRect.top()));
    QPointF br = worldToScreen(QPointF(m_printWindowRect.right(), m_printWindowRect.bottom()));
    QPen pen(QColor(255, 0, 0));
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.setBrush(QColor(255, 0, 0, 30));
    painter.drawRect(QRectF(tl, br));
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF worldPos = screenToWorld(event->pos());
    m_lastWorldPos = worldPos;
    emit cursorPositionChanged(worldPos);

    if (m_selectingPrintWindow) {
        m_printWindowStart = worldPos;
        m_printWindowRect = QRectF();
        return;
    }

    if (event->button() == Qt::MiddleButton || m_currentTool == "pan") {
        m_panning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (m_currentTool == "select") {
        QString devId = findDeviceAt(worldPos);
        if (!devId.isEmpty()) {
            m_selectedDeviceId = devId;
            emit deviceSelected(devId);
            m_dragging = true;
            m_lastMousePos = event->pos();
        } else {
            m_selectedDeviceId.clear();
            emit deviceSelected("");
        }
        update();
    } else if (m_currentTool == "place" && !m_placeModelId.isEmpty()) {
        if (!m_project || m_project->floors.empty()) return;
        int floorIdx = findFloorAt(worldPos);
        if (floorIdx < 0) floorIdx = m_activeFloorIndex;
        m_activeFloorIndex = floorIdx;
        emit activeFloorChanged(floorIdx);

        emit projectAboutToChange();
        zf::DeviceInstance dev;
        dev.instanceId = "DEV_" + std::to_string(m_project->floors[floorIdx].devices.size() + 1);
        dev.modelId = m_placeModelId.toStdString();
        double ox = m_project->floors[floorIdx].origin.x;
        double oy = m_project->floors[floorIdx].origin.y;
        dev.position.x = worldPos.x() - ox;
        dev.position.y = worldPos.y() - oy;
        m_project->floors[floorIdx].devices.push_back(dev);
        emit projectChanged("放置器件");
        update();
    } else if (m_currentTool == "wall") {
        m_drawingWall = true;
        m_wallStartPoint = snapPoint(worldPos);
        m_wallPreviewPoint = m_wallStartPoint;
    } else if (m_currentTool == "cable") {
        QString devId = findDeviceAt(worldPos);
        if (!devId.isEmpty()) {
            m_drawingCable = true;
            m_cableStartDeviceId = devId;
            m_cablePreviewPoint = worldPos;
        }
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPointF worldPos = screenToWorld(event->pos());
    m_lastWorldPos = worldPos;
    emit cursorPositionChanged(worldPos);

    if (m_selectingPrintWindow) {
        m_printWindowRect = QRectF(m_printWindowStart, worldPos).normalized();
        update();
        return;
    }

    if (m_panning) {
        QPointF delta = event->pos() - m_lastMousePos;
        m_panOffset += delta;
        m_lastMousePos = event->pos();
        update();
        return;
    }

    if (m_dragging && !m_selectedDeviceId.isEmpty()) {
        QPointF delta = (worldPos - screenToWorld(m_lastMousePos));
        m_lastMousePos = event->pos();
        if (m_project) {
            for (auto& floor : m_project->floors) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == m_selectedDeviceId.toStdString()) {
                        dev.position.x += delta.x();
                        dev.position.y += delta.y();
                        update();
                        return;
                    }
                }
            }
        }
    }

    if (m_drawingWall) {
        m_wallPreviewPoint = snapPoint(worldPos);
        if (m_orthoEnabled) {
            double dx = m_wallPreviewPoint.x() - m_wallStartPoint.x();
            double dy = m_wallPreviewPoint.y() - m_wallStartPoint.y();
            if (fabs(dx) > fabs(dy)) {
                m_wallPreviewPoint.setY(m_wallStartPoint.y());
            } else {
                m_wallPreviewPoint.setX(m_wallStartPoint.x());
            }
        }
        update();
    }

    if (m_drawingCable) {
        m_cablePreviewPoint = worldPos;
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_selectingPrintWindow) {
        m_selectingPrintWindow = false;
        if (!m_printWindowRect.isNull() && m_printWindowRect.width() > 100 && m_printWindowRect.height() > 100) {
            emit printWindowSelected(m_printWindowRect);
        }
        emit statusMessage("打印窗口选择完成");
        update();
        return;
    }

    if (m_panning) {
        m_panning = false;
        setCursor(m_currentTool == "pan" ? Qt::OpenHandCursor : Qt::ArrowCursor);
        return;
    }

    if (m_dragging) {
        m_dragging = false;
        return;
    }

    if (m_drawingWall) {
        m_drawingWall = false;
        if (m_project && !m_project->floors.empty()) {
            int floorIdx = findFloorAt(m_wallStartPoint);
            if (floorIdx < 0) floorIdx = m_activeFloorIndex;
            double ox = m_project->floors[floorIdx].origin.x;
            double oy = m_project->floors[floorIdx].origin.y;
            emit projectAboutToChange();
            zf::Wall wall;
            wall.wallId = "WALL_" + std::to_string(m_project->floors[floorIdx].walls.size() + 1);
            wall.points.push_back({m_wallStartPoint.x() - ox, m_wallStartPoint.y() - oy});
            wall.points.push_back({m_wallPreviewPoint.x() - ox, m_wallPreviewPoint.y() - oy});
            wall.thickness_mm = 240;
            m_project->floors[floorIdx].walls.push_back(wall);
            emit projectChanged("绘制墙体");
        }
        update();
    }

    if (m_drawingCable) {
        m_drawingCable = false;
        QString targetId = findDeviceAt(screenToWorld(event->pos()));
        if (!targetId.isEmpty() && targetId != m_cableStartDeviceId && m_project) {
            emit projectAboutToChange();
            for (auto& floor : m_project->floors) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == m_cableStartDeviceId.toStdString()) {
                        zf::DeviceInstance::Connection conn;
                        conn.targetInstanceId = targetId.toStdString();
                        dev.connections.push_back(conn);
                        break;
                    }
                }
            }
            emit projectChanged("连接馈线");
        }
        m_cableStartDeviceId.clear();
        update();
    }
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_project) return;
    QPointF worldPos = screenToWorld(event->pos());
    for (int i = 0; i < (int)m_project->floors.size(); i++) {
        QRectF bounds = getFloorBounds(i);
        QRectF labelArea(bounds.left(), bounds.top() - 1500, bounds.width(), 1500);
        if (labelArea.contains(worldPos)) {
            goToFloor(i);
            emit statusMessage(QString("已定位到楼层 %1").arg(i + 1));
            return;
        }
    }
    if (!m_project->floors.empty()) {
        QRectF allBounds = getAllContentBounds();
        double sysX = allBounds.right() - 40000;
        QRectF sysLabelArea(sysX, allBounds.top() - 1500, 40000, 1500);
        if (sysLabelArea.contains(worldPos)) {
            goToSystemDiagram();
            emit statusMessage("已定位到系统图区域");
            return;
        }
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    QPointF screenPos = event->pos();
    QPointF worldBefore = screenToWorld(screenPos);
    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
    m_zoom *= factor;
    if (m_zoom < 0.01) m_zoom = 0.01;
    if (m_zoom > 100) m_zoom = 100;
    QPointF worldAfter = screenToWorld(screenPos);
    m_panOffset += QPointF((worldAfter.x() - worldBefore.x()) * m_zoom,
                           (worldAfter.y() - worldBefore.y()) * m_zoom);
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedDevice();
    } else if (event->key() == Qt::Key_Escape) {
        m_drawingWall = false;
        m_drawingCable = false;
        m_selectingPrintWindow = false;
        m_selectedDeviceId.clear();
        emit deviceSelected("");
        update();
    } else if (event->key() == Qt::Key_F8) {
        m_orthoEnabled = !m_orthoEnabled;
        emit statusMessage(m_orthoEnabled ? "正交模式: 开" : "正交模式: 关");
    } else if (event->key() == Qt::Key_F3) {
        m_snapEnabled = !m_snapEnabled;
        emit statusMessage(m_snapEnabled ? "捕捉: 开" : "捕捉: 关");
    }
}

QString CanvasWidget::findDeviceAt(const QPointF& worldPos)
{
    if (!m_project) return QString();
    double threshold = 15 / m_zoom;
    for (const auto& floor : m_project->floors) {
        double ox = floor.origin.x;
        double oy = floor.origin.y;
        for (const auto& dev : floor.devices) {
            double dx = dev.position.x + ox - worldPos.x();
            double dy = dev.position.y + oy - worldPos.y();
            if (sqrt(dx*dx + dy*dy) < threshold) {
                return QString::fromStdString(dev.instanceId);
            }
        }
    }
    return QString();
}

void CanvasWidget::startPrintWindowSelection()
{
    m_selectingPrintWindow = true;
    m_printWindowRect = QRectF();
    emit statusMessage("拖拽选择打印窗口区域...");
}
