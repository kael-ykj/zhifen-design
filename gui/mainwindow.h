#pragma once
#include <QMainWindow>
#include <QSettings>
#include <QStringList>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include "core/zf_types.h"
#include "device/device_library.h"
#include "mode_control/mode_control_layer.h"
#include "undo/undo_redo_stack.h"
#include "undo/project_snapshot_transaction.h"
#include "engine/auto_placer.h"

class CanvasWidget;
class DeviceListPanel;
class PropertyPanel;
class LayerPanel;
class CommandLine;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewProject();
    zf::Project generateTemplate(const QString& templateName);
    void onOpenProject();
    void onSaveProject();
    void onModeToggle();
    void onToolSelect();
    void onRunLinkCalc();
    void onRunSimulation();
    void onToggleHeatmap();
    void onShowSystemDiagram();
    void onCostEstimate();
    void onAutoPlace();
    void onGenerateReport();
    void onExportMaterialList();
    void onBatchExportDxf();
    void onExportDxf();
    void onExportImage();
    void onPrintPreview();
    void onPrint();
    void onPrintWindow();
    void onBatchPrint();
    void onPrintWindowSelected(const QRectF& windowRect);
    void onZoomIn();
    void onZoomOut();
    void onZoomFit();
    void onDeleteSelected();
    void onUndo();
    void onRedo();
    void onAbout();
    void onDeviceSelected(const QString& deviceId);
    void onStatusMessage(const QString& msg);
    void onPropertyChanged();
    void onDeviceDeleted(const QString& deviceId);
    void onFloorChanged(int index);
    void onCursorPositionChanged(const QPointF& worldPos);
    void onActiveFloorChanged(int floorIndex);
    void onCommandEntered(const QString& command);
    void onAddFloor();
    void onDeleteFloor();
    void onCloneFloor();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockPanels();
    void initProject();
    void updateModeIndicator();
    void refreshFloorCombo();
    void updateUndoButtons();
    void loadRecentFiles();
    void saveRecentFiles();
    void addRecentFile(const QString& path);
    void updateRecentMenu();
    void openRecentFile();
    void onProjectAboutToChange();
    void onProjectChanged(const QString& description);

    zf::Project m_project;
    zf::DeviceLibrary m_devLib;
    zf::ModeControlLayer m_modeLayer;
    zf::UndoRedoDoubleStack m_undoStack;
    zf::Project m_undoSnapshot;
    QSettings m_settings;
    QStringList m_recentFiles;
    QMenu* m_recentMenu{nullptr};

    CanvasWidget* m_canvas{nullptr};
    DeviceListPanel* m_devicePanel{nullptr};
    PropertyPanel* m_propertyPanel{nullptr};
    LayerPanel* m_layerPanel{nullptr};
    CommandLine* m_commandLine{nullptr};

    QAction* m_actNew{nullptr};
    QAction* m_actOpen{nullptr};
    QAction* m_actSave{nullptr};
    QAction* m_actUndo{nullptr};
    QAction* m_actRedo{nullptr};
    QAction* m_actDelete{nullptr};
    QAction* m_actMode{nullptr};
    QAction* m_actLinkCalc{nullptr};
    QAction* m_actSimulate{nullptr};
    QAction* m_actHeatmap{nullptr};
    QAction* m_actSystemDiagram{nullptr};
    QAction* m_actCostEstimate{nullptr};
    QAction* m_actAutoPlace{nullptr};
    QAction* m_actGenerateReport{nullptr};
    QAction* m_actExportMaterial{nullptr};
    QAction* m_actBatchExportDxf{nullptr};
    QAction* m_actExportImage{nullptr};
    QAction* m_actPrintPreview{nullptr};
    QAction* m_actPrint{nullptr};
    QAction* m_actPrintWindow{nullptr};
    QAction* m_actBatchPrint{nullptr};
    QAction* m_actExport{nullptr};
    QAction* m_actZoomIn{nullptr};
    QAction* m_actZoomOut{nullptr};
    QAction* m_actZoomFit{nullptr};
    QAction* m_actAbout{nullptr};
    QAction* m_actToolSelect{nullptr};
    QAction* m_actToolPlace{nullptr};
    QAction* m_actToolWall{nullptr};
    QAction* m_actToolCable{nullptr};
    QAction* m_actToolFeeder{nullptr};
    QAction* m_actInsertSplitter{nullptr};
    QAction* m_actInsertCoupler{nullptr};

    // 楼层管理
    QComboBox* m_floorCombo{nullptr};
    QAction* m_actAddFloor{nullptr};
    QAction* m_actDeleteFloor{nullptr};
    QAction* m_actCloneFloor{nullptr};

    bool m_heatmapVisible{false};

    // CAD状态栏组件
    QLabel* m_statusCoord{nullptr};
    QLabel* m_statusSnap{nullptr};
    QLabel* m_statusOrtho{nullptr};
    QLabel* m_statusScale{nullptr};
    QLabel* m_statusLayer{nullptr};
};
