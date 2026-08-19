#include "canvaswidget.h"
#include <QPaintEvent>
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

void CanvasWidget::refresh()
{
    update();
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    if (!m_project || m_project->floors.empty()) return;

    drawGrid(painter);
    drawWalls(painter);
    drawCables(painter);
    drawDevices(painter);
    drawSelectedHighlight(painter);

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

    // 缩放指示
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

    for (double x = startX; x < width(); x += gridSize) {
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));
    }
    for (double y = startY; y < height(); y += gridSize) {
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
    }

    // 坐标轴
    painter.setPen(QColor(180, 180, 180));
    painter.drawLine(origin, QPointF(width(), origin.y()));
    painter.drawLine(origin, QPointF(origin.x(), height()));
}

void CanvasWidget::drawWalls(QPainter& painter)
{
    if (!m_project) return;
    const auto& floor = m_project->floors[0];

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
    const auto& floor = m_project->floors[0];

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
    const auto& floor = m_project->floors[0];

    for (const auto& dev : floor.devices) {
        QPointF pos = worldToScreen(QPointF(dev.position.x, dev.position.y));
        double r = 12 * m_zoom;
        if (r < 4) r = 4;

        // 根据类型画不同颜色
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
                default: color = QColor(100, 100, 100); label = "器件"; break;
            }
        } else {
            color = QColor(100, 100, 100);
            label = "?";
        }

        painter.setBrush(color);
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(pos, r, r);

        // 标签
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", 8));
        painter.drawText(pos + QPointF(r + 2, 4), QString::fromStdString(dev.instanceId));

        // 功率标注（正式模式）
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
    const auto& floor = m_project->floors[0];

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
    const auto& floor = m_project->floors[0];

    double threshold = 15.0 / m_zoom;
    for (const auto& dev : floor.devices) {
        double dx = dev.position.x - worldPos.x();
        double dy = dev.position.y - worldPos.y();
        if (std::sqrt(dx*dx + dy*dy) < threshold) {
            return QString::fromStdString(dev.instanceId);
        }
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
        dev.instanceId = "DEV_" + std::to_string(m_project->floors[0].devices.size() + 1);
        dev.modelId = m_placeModelId.toStdString();
        dev.position = {worldPos.x(), worldPos.y()};
        m_project->floors[0].devices.push_back(dev);
        emit statusMessage("已放置: " + m_placeModelId + " at (" +
            QString::number(worldPos.x(), 'f', 0) + ", " +
            QString::number(worldPos.y(), 'f', 0) + ")");
        update();
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
        auto& floor = m_project->floors[0];
        for (auto& dev : floor.devices) {
            if (dev.instanceId == m_selectedDeviceId.toStdString()) {
                dev.position = {worldPos.x(), worldPos.y()};
                break;
            }
        }
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
        m_dragging = false;
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event)
{
    double factor = event->angleDelta().y() > 0 ? 1.15 : 0.87;
    double newZoom = m_zoom * factor;
    if (newZoom < 0.1) newZoom = 0.1;
    if (newZoom > 20) newZoom = 20;

    // 以鼠标位置为中心缩放
    QPointF mousePos = event->pos();
    QPointF worldBefore = screenToWorld(mousePos);
    m_zoom = newZoom;
    QPointF worldAfter = screenToWorld(mousePos);
    m_panOffset += (worldAfter - worldBefore) * m_zoom;

    update();
}
