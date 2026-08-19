#include "mainwindow.h"
#include "canvaswidget.h"
#include "devicelistpanel.h"
#include "propertypanel.h"
#include "engine/link_calculator.h"
#include "engine/propagation_engine.h"
#include "io/drawing_exporter.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("智分Design V3.1.0 - 室分设计软件");
    resize(1280, 800);

    initProject();
    createActions();
    createMenus();
    createToolBars();
    createDockPanels();

    // 中央画布
    m_canvas = new CanvasWidget(this);
    m_canvas->setProject(&m_project);
    setCentralWidget(m_canvas);

    // 状态栏
    statusBar()->showMessage("就绪 | 当前模式: 草图模式");

    connect(m_canvas, &CanvasWidget::deviceSelected,
            this, &MainWindow::onDeviceSelected);
    connect(m_canvas, &CanvasWidget::statusMessage,
            this, &MainWindow::onStatusMessage);
}

MainWindow::~MainWindow() {}

void MainWindow::initProject()
{
    m_project.projectId = "GUI_PROJECT";
    m_project.projectName = "未命名工程";
    m_devLib.loadDefaultLibrary();

    // 复制器件库到项目
    for (const auto& cat : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                            zf::DeviceCategory::COUPLER, zf::DeviceCategory::ANTENNA,
                            zf::DeviceCategory::CABLE, zf::DeviceCategory::COMBINER}) {
        auto models = m_devLib.getModelsByCategory(cat);
        for (const auto& m : models) m_project.deviceLibrary.push_back(m);
    }

    // 默认楼层
    zf::Floor floor;
    floor.floorId = "F1";
    floor.floorName = "1F";
    m_project.floors.push_back(floor);

    m_modeLayer.init();
}

void MainWindow::createActions()
{
    m_actNew = new QAction("新建工程", this);
    m_actNew->setShortcut(QKeySequence::New);
    connect(m_actNew, &QAction::triggered, this, &MainWindow::onNewProject);

    m_actOpen = new QAction("打开工程", this);
    m_actOpen->setShortcut(QKeySequence::Open);
    connect(m_actOpen, &QAction::triggered, this, &MainWindow::onOpenProject);

    m_actSave = new QAction("保存工程", this);
    m_actSave->setShortcut(QKeySequence::Save);
    connect(m_actSave, &QAction::triggered, this, &MainWindow::onSaveProject);

    m_actMode = new QAction("切换正式模式", this);
    m_actMode->setCheckable(true);
    connect(m_actMode, &QAction::triggered, this, &MainWindow::onModeToggle);

    m_actLinkCalc = new QAction("链路预算", this);
    connect(m_actLinkCalc, &QAction::triggered, this, &MainWindow::onRunLinkCalc);

    m_actSimulate = new QAction("覆盖仿真", this);
    connect(m_actSimulate, &QAction::triggered, this, &MainWindow::onRunSimulation);

    m_actExport = new QAction("导出DXF", this);
    connect(m_actExport, &QAction::triggered, this, &MainWindow::onExportDxf);

    m_actAbout = new QAction("关于", this);
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::onAbout);

    m_actToolSelect = new QAction("选择", this);
    m_actToolSelect->setCheckable(true);
    m_actToolSelect->setChecked(true);
    connect(m_actToolSelect, &QAction::triggered, this, &MainWindow::onToolSelect);

    m_actToolPlace = new QAction("放置器件", this);
    m_actToolPlace->setCheckable(true);
    connect(m_actToolPlace, &QAction::triggered, this, &MainWindow::onToolSelect);

    m_actToolWall = new QAction("绘制墙体", this);
    m_actToolWall->setCheckable(true);
    connect(m_actToolWall, &QAction::triggered, this, &MainWindow::onToolSelect);

    m_actToolCable = new QAction("绘制线缆", this);
    m_actToolCable->setCheckable(true);
    connect(m_actToolCable, &QAction::triggered, this, &MainWindow::onToolSelect);
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction(m_actNew);
    fileMenu->addAction(m_actOpen);
    fileMenu->addAction(m_actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actExport);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu("编辑");
    QAction* undoAct = new QAction("撤销", this);
    undoAct->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAct);
    QAction* redoAct = new QAction("重做", this);
    redoAct->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAct);

    QMenu* calcMenu = menuBar()->addMenu("计算");
    calcMenu->addAction(m_actLinkCalc);
    calcMenu->addAction(m_actSimulate);

    QMenu* viewMenu = menuBar()->addMenu("视图");
    viewMenu->addAction(m_actMode);

    QMenu* helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBars()
{
    QToolBar* fileTool = addToolBar("文件");
    fileTool->addAction(m_actNew);
    fileTool->addAction(m_actOpen);
    fileTool->addAction(m_actSave);

    QToolBar* toolBar = addToolBar("工具");
    toolBar->addAction(m_actToolSelect);
    toolBar->addAction(m_actToolPlace);
    toolBar->addAction(m_actToolWall);
    toolBar->addAction(m_actToolCable);
    toolBar->addSeparator();
    toolBar->addAction(m_actMode);

    QToolBar* calcTool = addToolBar("计算");
    calcTool->addAction(m_actLinkCalc);
    calcTool->addAction(m_actSimulate);
    calcTool->addAction(m_actExport);
}

void MainWindow::createDockPanels()
{
    // 器件库面板
    QDockWidget* deviceDock = new QDockWidget("器件库", this);
    m_devicePanel = new DeviceListPanel(&m_devLib, deviceDock);
    deviceDock->setWidget(m_devicePanel);
    addDockWidget(Qt::LeftDockWidgetArea, deviceDock);

    // 属性面板
    QDockWidget* propDock = new QDockWidget("属性", this);
    m_propertyPanel = new PropertyPanel(propDock);
    propDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propDock);

    connect(m_devicePanel, &DeviceListPanel::deviceModelSelected,
            m_canvas, &CanvasWidget::setPlaceModel);
}

void MainWindow::onNewProject()
{
    m_project.floors.clear();
    zf::Floor floor;
    floor.floorId = "F1";
    floor.floorName = "1F";
    m_project.floors.push_back(floor);
    m_canvas->refresh();
    statusBar()->showMessage("新建工程完成");
}

void MainWindow::onOpenProject()
{
    QMessageBox::information(this, "打开工程", "工程文件打开功能开发中");
}

void MainWindow::onSaveProject()
{
    QMessageBox::information(this, "保存工程", "工程保存功能开发中");
}

void MainWindow::onModeToggle()
{
    auto* modeMgr = m_modeLayer.modeManager.get();
    if (m_actMode->isChecked()) {
        modeMgr->setGlobalWorkMode(zf::WorkMode::FORMAL_MODE, "user");
        m_actMode->setText("切换草图模式");
        statusBar()->showMessage("已切换到正式工程模式");
    } else {
        modeMgr->setGlobalWorkMode(zf::WorkMode::SKETCH_MODE, "user");
        m_actMode->setText("切换正式模式");
        statusBar()->showMessage("已切换到草图模式");
    }
    m_canvas->setModeManager(modeMgr);
}

void MainWindow::onToolSelect()
{
    if (m_actToolSelect->isChecked()) {
        m_canvas->setCurrentTool("select");
        statusBar()->showMessage("当前工具: 选择");
    } else if (m_actToolPlace->isChecked()) {
        m_canvas->setCurrentTool("place");
        statusBar()->showMessage("当前工具: 放置器件");
    } else if (m_actToolWall->isChecked()) {
        m_canvas->setCurrentTool("wall");
        statusBar()->showMessage("当前工具: 绘制墙体");
    } else if (m_actToolCable->isChecked()) {
        m_canvas->setCurrentTool("cable");
        statusBar()->showMessage("当前工具: 绘制线缆");
    }
}

void MainWindow::onRunLinkCalc()
{
    zf::LinkCalculator calc;
    calc.setModeManager(m_modeLayer.modeManager.get());
    int result = calc.calculateProject(&m_project);
    if (result == zf::ZF_ERR_OK) {
        const auto& report = calc.getReport();
        QString msg = QString("链路预算完成!\n器件: %1/%2\n错误: %3 警告: %4\n"
                              "天线功率: %5 ~ %6 dBm\n覆盖率相关: 平均 %7 dBm")
            .arg(report.calculatedDevices).arg(report.totalDevices)
            .arg(report.errorCount).arg(report.warningCount)
            .arg(report.minAntennaPower_dBm).arg(report.maxAntennaPower_dBm)
            .arg(report.avgAntennaPower_dBm);
        QMessageBox::information(this, "链路预算", msg);
        m_canvas->refresh();
    } else {
        QMessageBox::warning(this, "链路预算", QString("计算失败: %1").arg(result));
    }
}

void MainWindow::onRunSimulation()
{
    QMessageBox::information(this, "覆盖仿真", "覆盖仿真热力图功能开发中");
}

void MainWindow::onExportDxf()
{
    QString path = QFileDialog::getSaveFileName(this, "导出DXF", "", "DXF Files (*.dxf)");
    if (path.isEmpty()) return;

    zf::DrawingExporter exporter;
    exporter.setModeManager(m_modeLayer.modeManager.get());
    int result = exporter.exportFloorPlanDxf(path.toStdString(), &m_project.floors[0]);
    if (result == zf::ZF_ERR_OK) {
        QMessageBox::information(this, "导出成功", "DXF文件已导出: " + path);
    } else {
        QMessageBox::warning(this, "导出失败", "导出失败，错误码: " + QString::number(result));
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于智分Design",
        "智分Design V3.1.0\n\n"
        "室内分布系统设计软件\n"
        "P2 GUI预览版\n\n"
        "功能模块:\n"
        "- P0 内核引擎\n"
        "- P1 专业算法(链路预算/系统图/仿真/出图/批量/标准层/插件)\n"
        "- P2 图形界面(开发中)");
}

void MainWindow::onDeviceSelected(const QString& deviceId)
{
    m_propertyPanel->setDeviceId(deviceId);
    statusBar()->showMessage("选中器件: " + deviceId);
}

void MainWindow::onStatusMessage(const QString& msg)
{
    statusBar()->showMessage(msg);
}
