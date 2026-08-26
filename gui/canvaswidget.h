#pragma once
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPointF>
#include <QPixmap>
#include <QRectF>
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
    void setActiveFloorIndex(int index);
    int activeFloorIndex() const { return m_activeFloorIndex; }
    void setHeatmap(const zf::HeatmapData& heatmap);
    void clearHeatmap();
    void deleteSelectedDevice();
    void refresh();
    QPixmap exportToImage(int width = 1600, int height = 1200);
    QString selectedDeviceId() const { return m_selectedDeviceId; }

    // 统一模型空间
    int findFloorAt(const QPointF& worldPos) const;
    void goToFloor(int floorIndex);
    void goToSystemDiagram();
    QRectF getAllContentBounds() const;
    QRectF getFloorBounds(int floorIndex) const;

    // 状态栏
    bool snapEnabled() const { return m_snapEnabled; }
    bool orthoEnabled() const { return m_orthoEnabled; }
    void setSnapEnabled(bool e) { m_snapEnabled = e; }
    void setOrthoEnabled(bool e) { m_orthoEnabled = e; }
    double currentScale() const { return m_currentScale; }
    QPointF lastWorldPos() const { return m_lastWorldPos; }

    // 打印窗口
    void startPrintWindowSelection();
    QRectF selectedPrintWindow() const { return m_printWindowRect; }

signals:
    void deviceSelected(const QString& deviceId);
    void statusMessage(const QString& message);
    void deviceDeleted(const QString& deviceId);
    void projectAboutToChange();
    void projectChanged(const QString& description);
    void cursorPositionChanged(const QPointF& worldPos);
    void activeFloorChanged(int floorIndex);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void drawGrid(QPainter& painter);
    void drawHeatmap(QPainter& painter);
    void drawFloorLabels(QPainter& painter);
    void drawFloorSeparators(QPainter& painter);
    void drawSystemDiagramArea(QPainter& painter);
    void drawWallsForFloor(QPainter& painter, const zf::Floor& floor);
    void drawDevicesForFloor(QPainter& painter, const zf::Floor& floor);
    void drawCablesForFloor(QPainter& painter, const zf::Floor& floor);
    void drawSelectedHighlight(QPainter& painter);
    void drawPrintWindowPreview(QPainter& painter);
    QColor rsrpToColor(double rsrp) const;
    QPointF worldToScreen(const QPointF& world) const;
    QPointF screenToWorld(const QPointF& screen) const;
    QString findDeviceAt(const QPointF& worldPos);
    QPointF snapPoint(const QPointF& worldPos) const;

    zf::Project* m_project{nullptr};
    zf::ModeManager* m_modeMgr{nullptr};
    int m_activeFloorIndex{0};  // 当前操作楼层（放置器件等）
    QString m_currentTool{"select"};
    QString m_placeModelId;
    QString m_selectedDeviceId;
    zf::HeatmapData m_heatmap;
    bool m_hasHeatmap{false};

    // 视图变换
    double m_zoom{1.0};
    QPointF m_panOffset{0, 0};
    QPointF m_lastMousePos;
    bool m_panning{false};
    bool m_dragging{false};

    // 绘图状态
    bool m_drawingWall{false};
    QPointF m_wallStartPoint;
    QPointF m_wallPreviewPoint;
    bool m_drawingCable{false};
    QString m_cableStartDeviceId;
    QPointF m_cablePreviewPoint;

    // CAD状态栏
    bool m_snapEnabled{true};
    bool m_orthoEnabled{false};
    double m_currentScale{100.0}; // 1:100
    QPointF m_lastWorldPos{0, 0};

    // 打印窗口选择
    bool m_selectingPrintWindow{false};
    QPointF m_printWindowStart;
    QRectF m_printWindowRect;
};
