#pragma once

#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include "core/zf_types.h"
#include "device/device_library.h"
#include "mode_control/mode_control_layer.h"

class CanvasWidget;
class DeviceListPanel;
class PropertyPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onModeToggle();
    void onToolSelect();
    void onRunLinkCalc();
    void onRunSimulation();
    void onExportDxf();
    void onAbout();
    void onDeviceSelected(const QString& deviceId);
    void onStatusMessage(const QString& msg);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockPanels();
    void initProject();
    void updateModeIndicator();

    // 核心组件
    zf::Project m_project;
    zf::DeviceLibrary m_devLib;
    zf::ModeControlLayer m_modeLayer;

    // GUI组件
    CanvasWidget* m_canvas{nullptr};
    DeviceListPanel* m_devicePanel{nullptr};
    PropertyPanel* m_propertyPanel{nullptr};

    // Actions
    QAction* m_actNew{nullptr};
    QAction* m_actOpen{nullptr};
    QAction* m_actSave{nullptr};
    QAction* m_actMode{nullptr};
    QAction* m_actLinkCalc{nullptr};
    QAction* m_actSimulate{nullptr};
    QAction* m_actExport{nullptr};
    QAction* m_actAbout{nullptr};
    QAction* m_actToolSelect{nullptr};
    QAction* m_actToolPlace{nullptr};
    QAction* m_actToolWall{nullptr};
    QAction* m_actToolCable{nullptr};
};
