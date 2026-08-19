#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPointF>
#include "core/zf_types.h"
#include "mode_control/mode_control_layer.h"

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
    void refresh();

signals:
    void deviceSelected(const QString& deviceId);
    void statusMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void drawGrid(QPainter& painter);
    void drawWalls(QPainter& painter);
    void drawDevices(QPainter& painter);
    void drawCables(QPainter& painter);
    void drawSelectedHighlight(QPainter& painter);
    QPointF worldToScreen(const QPointF& world) const;
    QPointF screenToWorld(const QPointF& screen) const;
    QString findDeviceAt(const QPointF& worldPos);

    zf::Project* m_project{nullptr};
    zf::ModeManager* m_modeMgr{nullptr};

    QString m_currentTool{"select"};
    QString m_placeModelId;
    QString m_selectedDeviceId;

    // 视图变换
    double m_zoom{1.0};
    QPointF m_panOffset{0, 0};
    QPointF m_lastMousePos;
    bool m_panning{false};
    bool m_dragging{false};
};
