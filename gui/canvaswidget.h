#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPointF>
#include "core/zf_types.h"
#include "mode_control/mode_control_layer.h"
#include "engine/propagation_engine.h"

class CanvasWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
    ~CanvasWidget();
    void setProject(zf::Project* project);
    void setModeManager(zf::ModeManager* mgr);
    void setCurrentTool(const QString& tool);
    void setPlaceModel(const QString& modelId);
    void setHeatmap(const zf::HeatmapData& heatmap);
    void clearHeatmap();
    void deleteSelectedDevice();
    void refresh();
    QString selectedDeviceId() const { return m_selectedDeviceId; }

signals:
    void deviceSelected(const QString& deviceId);
    void statusMessage(const QString& message);
    void deviceDeleted(const QString& deviceId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void drawGrid(QPainter& painter);
    void drawHeatmap(QPainter& painter);
    void drawWalls(QPainter& painter);
    void drawDevices(QPainter& painter);
    void drawCables(QPainter& painter);
    void drawSelectedHighlight(QPainter& painter);
    QColor rsrpToColor(double rsrp) const;
    QPointF worldToScreen(const QPointF& world) const;
    QPointF screenToWorld(const QPointF& screen) const;
    QString findDeviceAt(const QPointF& worldPos);

    zf::Project* m_project{nullptr};
    zf::ModeManager* m_modeMgr{nullptr};
    QString m_currentTool{"select"};
    QString m_placeModelId;
    QString m_selectedDeviceId;
    zf::HeatmapData m_heatmap;
    bool m_hasHeatmap{false};
    double m_zoom{1.0};
    QPointF m_panOffset{0, 0};
    QPointF m_lastMousePos;
    bool m_panning{false};
    bool m_dragging{false};
    bool m_drawingWall{false};
    QPointF m_wallStartPoint;
    QPointF m_wallPreviewPoint;
    // 线缆连接
    bool m_drawingCable{false};
    QString m_cableStartDeviceId;
    QPointF m_cablePreviewPoint;
};
