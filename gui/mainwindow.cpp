#include "mainwindow.h"
#include "canvaswidget.h"
#include "devicelistpanel.h"
#include "propertypanel.h"
#include "system_dialog.h"
#include "engine/link_calculator.h"
#include "engine/propagation_engine.h"
#include "engine/system_diagram_engine.h"
#include "engine/cost_estimator.h"
#include "engine/report_generator.h"
#include "engine/floor_cloner.h"
#include "io/drawing_exporter.h"
#include "io/project_io.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QInputDialog>
#include <QLabel>
#include <QApplication>
#include <QWheelEvent>
#include <QFile>
#include <QTextStream>
#include <algorithm>

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
    refreshFloorCombo();

    m_canvas = new CanvasWidget(this);
    m_canvas->setProject(&m_project);
    setCentralWidget(m_canvas);

    statusBar()->showMessage("就绪 | 当前模式: 草图模式 | 提示: 左键选择/放置, 中键平移, 滚轮缩放, Delete删除");

    connect(m_canvas, &CanvasWidget::deviceSelected, this, &MainWindow::onDeviceSelected);
    connect(m_canvas, &CanvasWidget::statusMessage, this, &MainWindow::onStatusMessage);
    connect(m_canvas, &CanvasWidget::deviceDeleted, this, &MainWindow::onDeviceDeleted);
    connect(m_canvas, &CanvasWidget::projectAboutToChange, this, &MainWindow::onProjectAboutToChange);
    connect(m_canvas, &CanvasWidget::projectChanged, this, &MainWindow::onProjectChanged);
    updateUndoButtons();
}

MainWindow::~MainWindow() {}
void MainWindow::initProject()
{
    m_project.projectId = "GUI_PROJECT";
    m_project.projectName = "未命名工程";
    m_devLib.loadDefaultLibrary();
    for (const auto& cat : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                            zf::DeviceCategory::COUPLER, zf::DeviceCategory::ANTENNA,
                            zf::DeviceCategory::CABLE, zf::DeviceCategory::COMBINER}) {
        auto models = m_devLib.getModelsByCategory(cat);
        for (const auto& m : models) m_project.deviceLibrary.push_back(m);
    }
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

    m_actUndo = new QAction("撤销", this);
    m_actUndo->setShortcut(QKeySequence::Undo);
    m_actUndo->setEnabled(false);
    connect(m_actUndo, &QAction::triggered, this, &MainWindow::onUndo);

    m_actRedo = new QAction("重做", this);
    m_actRedo->setShortcut(QKeySequence::Redo);
    m_actRedo->setEnabled(false);
    connect(m_actRedo, &QAction::triggered, this, &MainWindow::onRedo);

    m_actDelete = new QAction("删除选中", this);
    m_actDelete->setShortcut(QKeySequence::Delete);
    connect(m_actDelete, &QAction::triggered, this, &MainWindow::onDeleteSelected);

    m_actMode = new QAction("切换正式模式", this);
    m_actMode->setCheckable(true);
    connect(m_actMode, &QAction::triggered, this, &MainWindow::onModeToggle);

    m_actLinkCalc = new QAction("链路预算", this);
    connect(m_actLinkCalc, &QAction::triggered, this, &MainWindow::onRunLinkCalc);

    m_actSimulate = new QAction("覆盖仿真", this);
    connect(m_actSimulate, &QAction::triggered, this, &MainWindow::onRunSimulation);

    m_actHeatmap = new QAction("热力图", this);
    m_actHeatmap->setCheckable(true);
    connect(m_actHeatmap, &QAction::triggered, this, &MainWindow::onToggleHeatmap);

    m_actSystemDiagram = new QAction("系统图", this);
    connect(m_actSystemDiagram, &QAction::triggered, this, &MainWindow::onShowSystemDiagram);

    m_actCostEstimate = new QAction("造价概算", this);
    connect(m_actCostEstimate, &QAction::triggered, this, &MainWindow::onCostEstimate);

    m_actAutoPlace = new QAction("自动布放", this);
    connect(m_actAutoPlace, &QAction::triggered, this, &MainWindow::onAutoPlace);

    m_actGenerateReport = new QAction("生成报告", this);
    connect(m_actGenerateReport, &QAction::triggered, this, &MainWindow::onGenerateReport);

    m_actExportMaterial = new QAction("材料表导出", this);
    connect(m_actExportMaterial, &QAction::triggered, this, &MainWindow::onExportMaterialList);

    m_actBatchExportDxf = new QAction("批量出图", this);
    connect(m_actBatchExportDxf, &QAction::triggered, this, &MainWindow::onBatchExportDxf);

    m_actExportImage = new QAction("导出图片", this);
    connect(m_actExportImage, &QAction::triggered, this, &MainWindow::onExportImage);

    m_actPrintPreview = new QAction("打印预览", this);
    connect(m_actPrintPreview, &QAction::triggered, this, &MainWindow::onPrintPreview);

    m_actPrint = new QAction("打印", this);
    m_actPrint->setShortcut(QKeySequence::Print);
    connect(m_actPrint, &QAction::triggered, this, &MainWindow::onPrint);


    m_actAddFloor = new QAction("新增楼层", this);
    connect(m_actAddFloor, &QAction::triggered, this, &MainWindow::onAddFloor);

    m_actDeleteFloor = new QAction("删除楼层", this);
    connect(m_actDeleteFloor, &QAction::triggered, this, &MainWindow::onDeleteFloor);

    m_actCloneFloor = new QAction("标准层复制", this);
    connect(m_actCloneFloor, &QAction::triggered, this, &MainWindow::onCloneFloor);
    m_actExport = new QAction("导出DXF", this);
    connect(m_actExport, &QAction::triggered, this, &MainWindow::onExportDxf);

    m_actZoomIn = new QAction("放大", this);
    m_actZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(m_actZoomIn, &QAction::triggered, this, &MainWindow::onZoomIn);

    m_actZoomOut = new QAction("缩小", this);
    m_actZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(m_actZoomOut, &QAction::triggered, this, &MainWindow::onZoomOut);

    m_actZoomFit = new QAction("适应窗口", this);
    connect(m_actZoomFit, &QAction::triggered, this, &MainWindow::onZoomFit);

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
    m_recentMenu = fileMenu->addMenu("最近打开");
    loadRecentFiles();
    updateRecentMenu();
    fileMenu->addAction(m_actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actExport);
    fileMenu->addAction(m_actExportImage);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actPrintPreview);
    fileMenu->addAction(m_actPrint);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction(m_actUndo);
    editMenu->addAction(m_actRedo);
    editMenu->addSeparator();
    editMenu->addAction(m_actDelete);

    QMenu* viewMenu = menuBar()->addMenu("视图");
    viewMenu->addAction(m_actZoomIn);
    viewMenu->addAction(m_actZoomOut);
    viewMenu->addAction(m_actZoomFit);
    viewMenu->addSeparator();
    viewMenu->addAction(m_actHeatmap);
    viewMenu->addAction(m_actMode);

    QMenu* calcMenu = menuBar()->addMenu("计算");
    calcMenu->addAction(m_actLinkCalc);
    calcMenu->addAction(m_actSimulate);
    calcMenu->addAction(m_actSystemDiagram);
    calcMenu->addSeparator();
    calcMenu->addAction(m_actCostEstimate);
    calcMenu->addAction(m_actAutoPlace);
    calcMenu->addAction(m_actGenerateReport);
    calcMenu->addSeparator();
    calcMenu->addAction(m_actExportMaterial);
    calcMenu->addAction(m_actBatchExportDxf);

    QMenu* helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBars()
{
    QToolBar* fileTool = addToolBar("文件");
    fileTool->addAction(m_actNew);
    fileTool->addAction(m_actOpen);
    fileTool->addAction(m_actSave);
    fileTool->addSeparator();
    fileTool->addAction(m_actExportImage);
    fileTool->addAction(m_actPrintPreview);
    fileTool->addAction(m_actPrint);

    // 楼层工具栏
    QToolBar* floorTool = addToolBar("楼层");
    QLabel* floorLabel = new QLabel(" 楼层: ", this);
    floorTool->addWidget(floorLabel);
    m_floorCombo = new QComboBox(this);
    m_floorCombo->setMinimumWidth(120);
    floorTool->addWidget(m_floorCombo);
    floorTool->addAction(m_actAddFloor);
    floorTool->addAction(m_actDeleteFloor);
    floorTool->addSeparator();
    floorTool->addAction(m_actCloneFloor);
    connect(m_floorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFloorChanged);

    QToolBar* editTool = addToolBar("编辑");
    editTool->addAction(m_actUndo);
    editTool->addAction(m_actRedo);
    editTool->addAction(m_actDelete);

    QToolBar* toolBar = addToolBar("工具");
    toolBar->addAction(m_actToolSelect);
    toolBar->addAction(m_actToolPlace);
    toolBar->addAction(m_actToolWall);
    toolBar->addAction(m_actToolCable);
    toolBar->addSeparator();
    toolBar->addAction(m_actMode);

    QToolBar* viewTool = addToolBar("视图");
    viewTool->addAction(m_actZoomIn);
    viewTool->addAction(m_actZoomOut);
    viewTool->addAction(m_actZoomFit);
    viewTool->addSeparator();
    viewTool->addAction(m_actHeatmap);

    QToolBar* calcTool = addToolBar("计算");
    calcTool->addAction(m_actLinkCalc);
    calcTool->addAction(m_actSimulate);
    calcTool->addAction(m_actSystemDiagram);
    calcTool->addSeparator();
    calcTool->addAction(m_actCostEstimate);
    calcTool->addAction(m_actGenerateReport);
    calcTool->addAction(m_actAutoPlace);
    calcTool->addSeparator();
    calcTool->addAction(m_actExportMaterial);
    calcTool->addAction(m_actBatchExportDxf);
    calcTool->addAction(m_actExport);
}

void MainWindow::createDockPanels()
{
    QDockWidget* deviceDock = new QDockWidget("器件库", this);
    m_devicePanel = new DeviceListPanel(&m_devLib, deviceDock);
    deviceDock->setWidget(m_devicePanel);
    addDockWidget(Qt::LeftDockWidgetArea, deviceDock);

    QDockWidget* propDock = new QDockWidget("属性", this);
    m_propertyPanel = new PropertyPanel(propDock);
    m_propertyPanel->setProject(&m_project);
    propDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, propDock);

    connect(m_devicePanel, &DeviceListPanel::deviceModelSelected,
            m_canvas, &CanvasWidget::setPlaceModel);
    connect(m_propertyPanel, &PropertyPanel::propertyChanged,
            this, &MainWindow::onPropertyChanged);
    connect(m_propertyPanel, &PropertyPanel::deviceDeleted,
            this, &MainWindow::onDeviceDeleted);
}

void MainWindow::onNewProject()
{
    QStringList templates = {"空白工程", "小型办公室", "大型商场", "酒店客房层"};
    bool ok = false;
    QString selected = QInputDialog::getItem(this, "新建工程", "选择工程模板:", templates, 0, false, &ok);
    if (!ok) return;

    m_project = generateTemplate(selected);
    m_canvas->clearHeatmap();
    m_heatmapVisible = false;
    m_actHeatmap->setChecked(false);
    m_canvas->refresh();
    m_propertyPanel->clear();
    refreshFloorCombo();
    setWindowTitle(QString("智分Design V3.1.0 - %1").arg(selected));
    statusBar()->showMessage("新建工程完成: " + selected);
}
void MainWindow::onSaveProject()
{
    QString path = QFileDialog::getSaveFileName(this, "保存工程", "", "智分工程文件 (*.zfp *.json)");
    if (path.isEmpty()) return;
    zf::ProjectIO io;
    int result = io.saveProject(path.toStdString(), &m_project);
    if (result == zf::ZF_ERR_OK) {
        statusBar()->showMessage("工程已保存: " + path);
        QMessageBox::information(this, "保存成功", "工程文件已保存:\n" + path);
    } else {
        QMessageBox::warning(this, "保存失败", QString("保存失败，错误码: %1").arg(result));
    }
}

void MainWindow::onOpenProject()
{
    QString path = QFileDialog::getOpenFileName(this, "打开工程", "", "智分工程文件 (*.zfp *.json)");
    if (path.isEmpty()) return;
    zf::ProjectIO io;
    auto loaded = io.loadProject(path.toStdString());
    if (loaded) {
        m_project = *loaded;
        // 确保器件库完整
        if (m_project.deviceLibrary.empty()) {
            for (const auto& cat : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                                    zf::DeviceCategory::COUPLER, zf::DeviceCategory::ANTENNA,
                                    zf::DeviceCategory::CABLE, zf::DeviceCategory::COMBINER}) {
                auto models = m_devLib.getModelsByCategory(cat);
                for (const auto& m : models) m_project.deviceLibrary.push_back(m);
            }
        }
        if (m_project.floors.empty()) {
            zf::Floor floor;
            floor.floorId = "F1";
            floor.floorName = "1F";
            m_project.floors.push_back(floor);
        }
        m_canvas->clearHeatmap();
        m_heatmapVisible = false;
        m_actHeatmap->setChecked(false);
        m_canvas->refresh();
        m_propertyPanel->clear();
        setWindowTitle(QString("智分Design V3.1.0 - %1").arg(QString::fromStdString(m_project.projectName)));
        refreshFloorCombo();
        statusBar()->showMessage("工程已加载: " + path);
        addRecentFile(path);
    } else {
        QMessageBox::warning(this, "打开失败", "无法打开工程文件，文件可能已损坏或格式不兼容");
    }
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
        statusBar()->showMessage("当前工具: 放置器件 (从左侧器件库选择)");
    } else if (m_actToolWall->isChecked()) {
        m_canvas->setCurrentTool("wall");
        statusBar()->showMessage("当前工具: 绘制墙体 (点击起点→点击终点，右键结束)");
    } else if (m_actToolCable->isChecked()) {
        m_canvas->setCurrentTool("cable");
        statusBar()->showMessage("当前工具: 线缆连接 (点击起点器件→点击目标器件，右键结束)");
    }
}

void MainWindow::onRunLinkCalc()
{
    if (m_project.floors.empty() || m_project.floors[m_canvas->currentFloorIndex()].devices.empty()) {
        QMessageBox::warning(this, "链路预算", "请先放置器件");
        return;
    }
    zf::LinkCalculator calc;
    calc.setModeManager(m_modeLayer.modeManager.get());
    int result = calc.calculateProject(&m_project);
    if (result == zf::ZF_ERR_OK) {
        const auto& report = calc.getReport();
        QString msg = QString("链路预算完成!\n\n器件: %1/%2\n错误: %3  警告: %4\n"
                              "天线功率: %5 ~ %6 dBm\n平均: %7 dBm")
            .arg(report.calculatedDevices).arg(report.totalDevices)
            .arg(report.errorCount).arg(report.warningCount)
            .arg(report.minAntennaPower_dBm).arg(report.maxAntennaPower_dBm)
            .arg(report.avgAntennaPower_dBm);
        QMessageBox::information(this, "链路预算", msg);
        m_canvas->refresh();
        m_propertyPanel->refresh();
        statusBar()->showMessage("链路预算完成");
    } else {
        QMessageBox::warning(this, "链路预算", QString("计算失败，错误码: %1").arg(result));
    }
}

void MainWindow::onRunSimulation()
{
    if (m_project.floors.empty() || m_project.floors[m_canvas->currentFloorIndex()].devices.empty()) {
        QMessageBox::warning(this, "覆盖仿真", "请先放置器件");
        return;
    }
    // 找到第一个天线作为发射点
    zf::Point2D txPos{0, 0};
    bool found = false;
    for (const auto& dev : m_project.floors[m_canvas->currentFloorIndex()].devices) {
        auto it = std::find_if(m_project.deviceLibrary.begin(), m_project.deviceLibrary.end(),
            [&](const zf::DeviceModel& m) { return m.modelId == dev.modelId; });
        if (it != m_project.deviceLibrary.end() && it->category == zf::DeviceCategory::ANTENNA) {
            txPos = dev.position;
            found = true;
            break;
        }
    }
    if (!found) {
        // 用第一个器件
        txPos = m_project.floors[m_canvas->currentFloorIndex()].devices[0].position;
    }

    zf::PropagationEngine engine;
    engine.setModeManager(m_modeLayer.modeManager.get());
    zf::SimulationConfig cfg;
    cfg.gridResolution_m = 1.0;
    cfg.maxDistance_m = 80.0;
    engine.setConfig(cfg);

    zf::HeatmapData heatmap;
    int result = engine.generateHeatmap(&m_project.floors[m_canvas->currentFloorIndex()], txPos, heatmap);
    if (result == zf::ZF_ERR_OK) {
        m_canvas->setHeatmap(heatmap);
        m_heatmapVisible = true;
        m_actHeatmap->setChecked(true);
        QString msg = QString("覆盖仿真完成!\n\n网格点: %1\n覆盖率(>=-100dBm): %2%\n"
                              "平均RSRP: %3 dBm\n范围: %4 ~ %5 dBm\n弱覆盖点: %6")
            .arg(heatmap.points.size())
            .arg(heatmap.coverageRate * 100, 0, 'f', 1)
            .arg(heatmap.avgRSRP, 0, 'f', 1)
            .arg(heatmap.minRSRP, 0, 'f', 1)
            .arg(heatmap.maxRSRP, 0, 'f', 1)
            .arg(heatmap.weakCoverageCount);
        QMessageBox::information(this, "覆盖仿真", msg);
        statusBar()->showMessage("覆盖仿真完成，热力图已显示");
    } else {
        QMessageBox::warning(this, "覆盖仿真", QString("仿真失败，错误码: %1 (草图模式下重型计算受限)").arg(result));
    }
}

void MainWindow::onToggleHeatmap()
{
    if (m_actHeatmap->isChecked()) {
        if (!m_heatmapVisible) {
            onRunSimulation();
        }
    } else {
        m_canvas->clearHeatmap();
        m_heatmapVisible = false;
        statusBar()->showMessage("热力图已隐藏");
    }
}

void MainWindow::onShowSystemDiagram()
{
    if (m_project.floors.empty() || m_project.floors[m_canvas->currentFloorIndex()].devices.empty()) {
        QMessageBox::warning(this, "系统图", "请先放置器件");
        return;
    }
    zf::SystemDiagramEngine engine;
    engine.setModeManager(m_modeLayer.modeManager.get());
    zf::SystemDiagram diagram;
    int result = engine.generateFromFloor(&m_project.floors[m_canvas->currentFloorIndex()], &m_project, diagram);
    if (result == zf::ZF_ERR_OK) {
        SystemDiagramDialog dlg(diagram, this);
        dlg.exec();
        statusBar()->showMessage("系统图已生成");
    } else {
        QMessageBox::warning(this, "系统图", QString("生成失败，错误码: %1 (需要信源器件和连接关系)").arg(result));
    }
}

void MainWindow::onCostEstimate()
{
    if (m_project.floors.empty()) {
        QMessageBox::warning(this, "造价概算", "工程为空");
        return;
    }
    zf::CostEstimator estimator;
    zf::CostSummary summary;
    int result = estimator.estimateProject(&m_project, summary);
    if (result == zf::ZF_ERR_OK) {
        // 按楼层分项统计
        QString floorBreakdown = "各楼层器件统计:\n";
        for (size_t i = 0; i < m_project.floors.size(); i++) {
            const auto& floor = m_project.floors[i];
            floorBreakdown += QString("  %1 %2: %3个器件, %4段墙体\n")
                .arg(QString::fromStdString(floor.floorId))
                .arg(QString::fromStdString(floor.floorName))
                .arg(floor.devices.size())
                .arg(floor.walls.size());
        }

        QString msg = QString("全楼工程造价概算完成!\n\n"
                              "楼层数:   %1 层\n"
                              "器件总数: %2 个\n"
                              "线缆总长: %3 米\n\n"
                              "%4\n"
                              "费用汇总:\n"
                              "  材料费:   %5 元\n"
                              "  线缆费:   %6 元\n"
                              "  人工费:   %7 元\n"
                              "  其他费:   %8 元\n"
                              "  小计:     %9 元\n"
                              "  税金(9%%): %10 元\n"
                              "  ========================\n"
                              "  含税总价: %11 元")
            .arg(m_project.floors.size())
            .arg(summary.deviceCount)
            .arg(summary.cableLength_m, 0, 'f', 1)
            .arg(floorBreakdown)
            .arg(summary.materialCost, 0, 'f', 2)
            .arg(summary.cableCost, 0, 'f', 2)
            .arg(summary.laborCost, 0, 'f', 2)
            .arg(summary.otherCost, 0, 'f', 2)
            .arg(summary.subtotal, 0, 'f', 2)
            .arg(summary.tax, 0, 'f', 2)
            .arg(summary.total, 0, 'f', 2);
        QMessageBox::information(this, "全楼造价概算", msg);
        statusBar()->showMessage(QString("全楼造价概算完成，含税总价: %1 元").arg(summary.total, 0, 'f', 2));
    } else {
        QMessageBox::warning(this, "造价概算", QString("概算失败，错误码: %1").arg(result));
    }
}

void MainWindow::onGenerateReport()
{
    if (m_project.floors.empty()) {
        QMessageBox::warning(this, "生成报告", "工程为空");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "保存设计报告", "", "HTML Files (*.html)");
    if (path.isEmpty()) return;

    // 先运行链路预算
    zf::LinkCalculator calc;
    calc.setModeManager(m_modeLayer.modeManager.get());
    zf::LinkReport linkReport;
    bool hasLinkReport = (calc.calculateProject(&m_project) == zf::ZF_ERR_OK);
    if (hasLinkReport) linkReport = calc.getReport();

    // 造价概算
    zf::CostEstimator estimator;
    zf::CostSummary costSummary;
    bool hasCost = (estimator.estimateProject(&m_project, costSummary) == zf::ZF_ERR_OK);

    // 生成报告
    zf::ReportGenerator reporter;
    zf::ReportConfig cfg;
    reporter.setConfig(cfg);

    std::string html;
    int result = reporter.generateHtmlReport(
        &m_project,
        hasLinkReport ? &linkReport : nullptr,
        hasCost ? &costSummary : nullptr,
        html);

    if (result == zf::ZF_ERR_OK) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(html.c_str(), html.size());
            file.close();
            QMessageBox::information(this, "报告生成", "设计报告已生成:\n" + path);
            statusBar()->showMessage("设计报告已生成: " + path);
        } else {
            QMessageBox::warning(this, "报告生成", "文件写入失败");
        }
    } else {
        QMessageBox::warning(this, "报告生成", QString("生成失败，错误码: %1").arg(result));
    }
}

void MainWindow::onExportMaterialList()
{
    if (m_project.floors.empty()) {
        QMessageBox::warning(this, "材料表导出", "工程为空");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "导出材料表", "", "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    zf::CostEstimator estimator;
    zf::CostSummary summary;
    estimator.estimateProject(&m_project, summary);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "材料表导出", "文件写入失败");
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    out << "序号,类别,型号,名称,数量,单价(元),总价(元),备注\n";
    int idx = 1;
    double total = 0;
    for (const auto& item : summary.items) {
        out << idx++ << ","
            << QString::fromStdString(item.category) << ","
            << QString::fromStdString(item.modelId) << ","
            << QString::fromStdString(item.itemName) << ","
            << item.quantity << ","
            << QString::number(item.unitPrice, 'f', 2) << ","
            << QString::number(item.totalPrice, 'f', 2) << ","
            << QString::fromStdString(item.remark) << "\n";
        total += item.totalPrice;
    }
    out << ",,,,,合计," << QString::number(total, 'f', 2) << ",\n";
    out << ",,,,,含税总价(9%)," << QString::number(summary.total, 'f', 2) << ",\n";
    file.close();

    QMessageBox::information(this, "材料表导出",
        QString("材料表已导出:\n%1\n\n共 %2 项，含税总价: %3 元")
        .arg(path).arg(summary.items.size()).arg(summary.total, 0, 'f', 2));
    statusBar()->showMessage("材料表已导出: " + path);
}

void MainWindow::onExportDxf()
{
    QString path = QFileDialog::getSaveFileName(this, "导出DXF", "", "DXF Files (*.dxf)");
    if (path.isEmpty()) return;
    zf::DrawingExporter exporter;
    exporter.setModeManager(m_modeLayer.modeManager.get());
    int result = exporter.exportFloorPlanDxf(path.toStdString(), &m_project.floors[m_canvas->currentFloorIndex()]);
    if (result == zf::ZF_ERR_OK) {
        QMessageBox::information(this, "导出成功", "DXF文件已导出: " + path);
    } else {
        QMessageBox::warning(this, "导出失败", "导出失败，错误码: " + QString::number(result));
    }
}

void MainWindow::onZoomIn()
{
    // 通过模拟滚轮事件实现缩放
    QWheelEvent event(QPointF(), QPointF(), QPoint(0, 0), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(m_canvas, &event);
}

void MainWindow::onZoomOut()
{
    QWheelEvent event(QPointF(), QPointF(), QPoint(0, 0), QPoint(0, -120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(m_canvas, &event);
}

void MainWindow::onZoomFit()
{
    statusBar()->showMessage("适应窗口功能开发中");
}

void MainWindow::onDeleteSelected()
{
    m_canvas->deleteSelectedDevice();
    m_propertyPanel->clear();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于智分Design",
        "智分Design V3.1.0\n\n"
        "室内分布系统设计软件\n"
        "P2 GUI增强版\n\n"
        "功能模块:\n"
        "- P0 内核引擎 (数据结构/错误码/告警/模式控制/撤销重做/器件库/工具系统/图形引擎/项目IO/DWG导出)\n"
        "- P1 专业算法 (链路预算/系统图/出图引擎/墙体建模/传播仿真/批量编辑/标准层复制/插件框架)\n"
        "- P2 图形界面 (主窗口/画布/器件库/属性面板/热力图/系统图/工具栏)\n\n"
        "操作说明:\n"
        "- 左键: 选择器件 / 放置器件\n"
        "- 中键拖动: 平移视图\n"
        "- 滚轮: 缩放视图\n"
        "- Delete: 删除选中器件\n"
        "- Esc: 取消选择/放置");
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

void MainWindow::onPropertyChanged()
{
    m_canvas->refresh();
    statusBar()->showMessage("属性已更新");
}

void MainWindow::onDeviceDeleted(const QString& deviceId)
{
    m_propertyPanel->clear();
    m_canvas->refresh();
    statusBar()->showMessage("已删除器件: " + deviceId);
}

void MainWindow::refreshFloorCombo()
{
    if (!m_floorCombo) return;
    m_floorCombo->blockSignals(true);
    m_floorCombo->clear();
    for (size_t i = 0; i < m_project.floors.size(); i++) {
        const auto& floor = m_project.floors[i];
        QString label = QString("%1 - %2").arg(QString::fromStdString(floor.floorId))
                                            .arg(QString::fromStdString(floor.floorName));
        if (floor.isStandardFloor) label += " [标准层]";
        m_floorCombo->addItem(label);
    }
    int idx = m_canvas ? m_canvas->currentFloorIndex() : 0;
    if (idx >= 0 && idx < m_floorCombo->count()) {
        m_floorCombo->setCurrentIndex(idx);
    }
    m_floorCombo->blockSignals(false);
    // 删除楼层按钮：只有1层时禁用
    if (m_actDeleteFloor) {
        m_actDeleteFloor->setEnabled(m_project.floors.size() > 1);
    }
}

void MainWindow::onFloorChanged(int index)
{
    if (m_canvas && index >= 0) {
        m_canvas->setCurrentFloorIndex(index);
        m_propertyPanel->clear();
        const auto& floor = m_project.floors[index];
        statusBar()->showMessage(QString("切换到楼层: %1 - %2 (器件: %3, 墙体: %4)")
            .arg(QString::fromStdString(floor.floorId))
            .arg(QString::fromStdString(floor.floorName))
            .arg(floor.devices.size())
            .arg(floor.walls.size()));
    }
}

void MainWindow::onAddFloor()
{
    bool ok;
    QString floorName = QInputDialog::getText(this, "新增楼层", "楼层名称:", QLineEdit::Normal,
        QString("F%1").arg(m_project.floors.size() + 1), &ok);
    if (!ok || floorName.isEmpty()) return;

    zf::Floor newFloor;
    newFloor.floorId = "F" + std::to_string(m_project.floors.size() + 1);
    newFloor.floorName = floorName.toStdString();
    newFloor.floorIndex = (int)m_project.floors.size();
    newFloor.height_m = 3.0;
    m_project.floors.push_back(newFloor);

    refreshFloorCombo();
    m_floorCombo->setCurrentIndex((int)m_project.floors.size() - 1);
    statusBar()->showMessage("已新增楼层: " + floorName);
}

void MainWindow::onDeleteFloor()
{
    if (m_project.floors.size() <= 1) {
        QMessageBox::warning(this, "删除楼层", "至少保留一个楼层");
        return;
    }
    int idx = m_floorCombo ? m_floorCombo->currentIndex() : 0;
    const auto& floor = m_project.floors[idx];
    if (QMessageBox::question(this, "确认删除",
        QString("确定要删除楼层 %1 - %2 吗？\n该楼层所有器件和墙体将被删除。")
        .arg(QString::fromStdString(floor.floorId))
        .arg(QString::fromStdString(floor.floorName))) != QMessageBox::Yes) {
        return;
    }
    m_project.floors.erase(m_project.floors.begin() + idx);
    if (idx >= (int)m_project.floors.size()) idx = (int)m_project.floors.size() - 1;
    refreshFloorCombo();
    m_floorCombo->setCurrentIndex(idx);
    statusBar()->showMessage("楼层已删除");
}

void MainWindow::onCloneFloor()
{
    if (m_project.floors.empty()) return;
    int srcIdx = m_floorCombo ? m_floorCombo->currentIndex() : 0;
    const auto& srcFloor = m_project.floors[srcIdx];

    bool ok;
    QString newName = QInputDialog::getText(this, "标准层复制",
        QString("将楼层 %1 复制为新楼层，新楼层名称:")
        .arg(QString::fromStdString(srcFloor.floorName)),
        QLineEdit::Normal, QString("F%1").arg(m_project.floors.size() + 1), &ok);
    if (!ok || newName.isEmpty()) return;

    zf::FloorCloner cloner;
    zf::CloneOptions options;
    options.cloneWalls = true;
    options.cloneDevices = true;
    options.cloneCables = true;
    options.idPrefix = newName.toStdString() + "_";
    options.offsetX = 0;
    options.offsetY = 0;

    zf::Floor newFloor;
    std::string newId = "F" + std::to_string(m_project.floors.size() + 1);
    int result = cloner.cloneFromStandard(&srcFloor, newId, newName.toStdString(), options, newFloor);
    if (result == zf::ZF_ERR_OK) {
        m_project.floors.push_back(newFloor);
        refreshFloorCombo();
        m_floorCombo->setCurrentIndex((int)m_project.floors.size() - 1);
        statusBar()->showMessage(QString("已从 %1 复制到 %2 (器件: %3, 墙体: %4)")
            .arg(QString::fromStdString(srcFloor.floorName))
            .arg(newName)
            .arg(newFloor.devices.size())
            .arg(newFloor.walls.size()));
    } else {
        QMessageBox::warning(this, "复制失败", QString("复制失败，错误码: %1").arg(result));
    }
}

void MainWindow::onBatchExportDxf()
{
    if (m_project.floors.empty()) {
        QMessageBox::warning(this, "批量出图", "工程为空，没有楼层可导出");
        return;
    }
    QString dir = QFileDialog::getExistingDirectory(this, "选择输出目录", "");
    if (dir.isEmpty()) return;

    zf::DrawingExporter exporter;
    exporter.setModeManager(m_modeLayer.modeManager.get());

    int success = 0, failed = 0;
    QStringList failedFloors;

    for (size_t i = 0; i < m_project.floors.size(); i++) {
        const auto& floor = m_project.floors[i];
        QString fileName = QString("%1_%2_平面图.dxf")
            .arg(QString::fromStdString(floor.floorId))
            .arg(QString::fromStdString(floor.floorName));
        // 清理文件名中的非法字符
        fileName.replace("/", "_").replace("\\", "_").replace(":", "_");
        QString filePath = dir + "/" + fileName;

        int result = exporter.exportFloorPlanDxf(filePath.toStdString(), &floor);
        if (result == zf::ZF_ERR_OK) {
            success++;
        } else {
            failed++;
            failedFloors << QString::fromStdString(floor.floorName);
        }
    }

    QString msg = QString("批量出图完成!\n\n成功: %1 层\n失败: %2 层\n输出目录: %3")
        .arg(success).arg(failed).arg(dir);
    if (!failedFloors.isEmpty()) {
        msg += "\n\n失败楼层: " + failedFloors.join(", ");
    }
    QMessageBox::information(this, "批量出图", msg);
    statusBar()->showMessage(QString("批量出图完成: 成功%1层, 失败%2层").arg(success).arg(failed));
}

void MainWindow::onProjectAboutToChange()
{
    m_undoSnapshot = m_project;
}

void MainWindow::onProjectChanged(const QString& description)
{
    auto tx = std::make_unique<zf::ProjectSnapshotTransaction>(
        &m_project, m_undoSnapshot, m_project, description.toStdString());
    m_undoStack.pushTransaction(std::move(tx));
    updateUndoButtons();
    m_propertyPanel->refresh();
}

void MainWindow::onUndo()
{
    if (m_undoStack.canUndo()) {
        m_undoStack.undo();
        m_canvas->refresh();
        m_propertyPanel->clear();
        updateUndoButtons();
        statusBar()->showMessage("已撤销");
    }
}

void MainWindow::onRedo()
{
    if (m_undoStack.canRedo()) {
        m_undoStack.redo();
        m_canvas->refresh();
        m_propertyPanel->clear();
        updateUndoButtons();
        statusBar()->showMessage("已重做");
    }
}

void MainWindow::updateUndoButtons()
{
    if (m_actUndo) m_actUndo->setEnabled(m_undoStack.canUndo());
    if (m_actRedo) m_actRedo->setEnabled(m_undoStack.canRedo());
}

void MainWindow::onExportImage()
{
    QString path = QFileDialog::getSaveFileName(this, "导出图片", "", "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (path.isEmpty()) return;

    QPixmap pixmap = m_canvas->exportToImage(2400, 1800);
    if (pixmap.save(path)) {
        QMessageBox::information(this, "导出成功", "图片已导出:\n" + path);
        statusBar()->showMessage("图片已导出: " + path);
    } else {
        QMessageBox::warning(this, "导出失败", "图片导出失败");
    }
}

void MainWindow::onPrintPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("打印预览");
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* printer) {
        QPainter painter(printer);
        QRect pageRect = printer->pageRect();
        // 计算缩放比例
        double scaleX = (double)pageRect.width() / m_canvas->width();
        double scaleY = (double)pageRect.height() / m_canvas->height();
        double scale = std::min(scaleX, scaleY);
        // 居中
        painter.translate(pageRect.x() + (pageRect.width() - m_canvas->width() * scale) / 2,
                            pageRect.y() + (pageRect.height() - m_canvas->height() * scale) / 2);
        painter.scale(scale, scale);
        m_canvas->render(&painter);
        painter.end();
    });
    preview.exec();
}

void MainWindow::onPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("打印");
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QRect pageRect = printer.pageRect();
        double scaleX = (double)pageRect.width() / m_canvas->width();
        double scaleY = (double)pageRect.height() / m_canvas->height();
        double scale = std::min(scaleX, scaleY);
        painter.translate(pageRect.x() + (pageRect.width() - m_canvas->width() * scale) / 2,
                            pageRect.y() + (pageRect.height() - m_canvas->height() * scale) / 2);
        painter.scale(scale, scale);
        m_canvas->render(&painter);
        painter.end();
        statusBar()->showMessage("已发送到打印机");
    }
}

void MainWindow::loadRecentFiles()
{
    m_recentFiles = m_settings.value("recentFiles", QStringList()).toStringList();
}

void MainWindow::saveRecentFiles()
{
    m_settings.setValue("recentFiles", m_recentFiles);
}

void MainWindow::addRecentFile(const QString& path)
{
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    if (m_recentFiles.size() > 10) m_recentFiles = m_recentFiles.mid(0, 10);
    saveRecentFiles();
    updateRecentMenu();
}

void MainWindow::updateRecentMenu()
{
    if (!m_recentMenu) return;
    m_recentMenu->clear();
    if (m_recentFiles.isEmpty()) {
        QAction* empty = m_recentMenu->addAction("（无最近文件）");
        empty->setEnabled(false);
        return;
    }
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        QString text = QString("%1. %2").arg(i + 1).arg(QFileInfo(m_recentFiles[i]).fileName());
        QAction* act = m_recentMenu->addAction(text);
        act->setData(m_recentFiles[i]);
        connect(act, &QAction::triggered, this, &MainWindow::openRecentFile);
    }
    m_recentMenu->addSeparator();
    QAction* clear = m_recentMenu->addAction("清空最近列表");
    connect(clear, &QAction::triggered, this, [this]() {
        m_recentFiles.clear();
        saveRecentFiles();
        updateRecentMenu();
    });
}

void MainWindow::openRecentFile()
{
    QAction* act = qobject_cast<QAction*>(sender());
    if (!act) return;
    QString path = act->data().toString();
    if (QFile::exists(path)) {
        zf::ProjectIO io;
        auto loaded = io.loadProject(path.toStdString());
        if (loaded) {
            m_project = *loaded;
            if (m_project.deviceLibrary.empty()) {
                for (const auto& cat : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                                        zf::DeviceCategory::COUPLER, zf::DeviceCategory::ANTENNA,
                                        zf::DeviceCategory::CABLE, zf::DeviceCategory::COMBINER}) {
                    auto models = m_devLib.getModelsByCategory(cat);
                    for (const auto& m : models) m_project.deviceLibrary.push_back(m);
                }
            }
            if (m_project.floors.empty()) {
                zf::Floor floor;
                floor.floorId = "F1";
                floor.floorName = "1F";
                m_project.floors.push_back(floor);
            }
            m_canvas->clearHeatmap();
            m_heatmapVisible = false;
            m_actHeatmap->setChecked(false);
            m_canvas->refresh();
            m_propertyPanel->clear();
            setWindowTitle(QString("智分Design V3.1.0 - %1").arg(QString::fromStdString(m_project.projectName)));
            refreshFloorCombo();
            addRecentFile(path);
            statusBar()->showMessage("工程已加载: " + path);
        } else {
            QMessageBox::warning(this, "打开失败", "无法打开工程文件，文件可能已损坏或格式不兼容");
            m_recentFiles.removeAll(path);
            saveRecentFiles();
            updateRecentMenu();
        }
    } else {
        QMessageBox::warning(this, "文件不存在", "该文件已被移动或删除: " + path);
        m_recentFiles.removeAll(path);
        saveRecentFiles();
        updateRecentMenu();
    }
}

zf::Project MainWindow::generateTemplate(const QString& templateName)
{
    zf::Project proj;
    proj.projectId = "TEMPLATE_" + templateName.toStdString();
    proj.projectName = templateName.toStdString();

    // 加载默认器件库
    for (const auto& cat : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                            zf::DeviceCategory::COUPLER, zf::DeviceCategory::ANTENNA,
                            zf::DeviceCategory::CABLE, zf::DeviceCategory::COMBINER}) {
        auto models = m_devLib.getModelsByCategory(cat);
        for (const auto& m : models) proj.deviceLibrary.push_back(m);
    }

    if (templateName == "空白工程") {
        zf::Floor floor;
        floor.floorId = "F1";
        floor.floorName = "1F";
        proj.floors.push_back(floor);
    }
    else if (templateName == "小型办公室") {
        zf::Floor floor;
        floor.floorId = "F1";
        floor.floorName = "1F 办公层";
        // 外墙
        zf::Wall w1, w2, w3, w4;
        w1.wallId = "WALL_EXT1"; w1.points = {{0,0},{800,0}}; w1.attenuation_dB = 15; w1.thickness_mm = 240;
        w2.wallId = "WALL_EXT2"; w2.points = {{800,0},{800,600}}; w2.attenuation_dB = 15; w2.thickness_mm = 240;
        w3.wallId = "WALL_EXT3"; w3.points = {{800,600},{0,600}}; w3.attenuation_dB = 15; w3.thickness_mm = 240;
        w4.wallId = "WALL_EXT4"; w4.points = {{0,600},{0,0}}; w4.attenuation_dB = 15; w4.thickness_mm = 240;
        // 内部隔墙
        zf::Wall w5, w6;
        w5.wallId = "WALL_INT1"; w5.points = {{400,0},{400,300}}; w5.attenuation_dB = 8; w5.thickness_mm = 120;
        w6.wallId = "WALL_INT2"; w6.points = {{0,300},{400,300}}; w6.attenuation_dB = 8; w6.thickness_mm = 120;
        floor.walls = {w1, w2, w3, w4, w5, w6};
        proj.floors.push_back(floor);
    }
    else if (templateName == "大型商场") {
        for (int f = 1; f <= 3; ++f) {
            zf::Floor floor;
            floor.floorId = "F" + std::to_string(f);
            floor.floorName = std::to_string(f) + "F 商场层";
            // 外墙
            zf::Wall w1, w2, w3, w4;
            w1.wallId = "WALL_F" + std::to_string(f) + "_EXT1"; w1.points = {{0,0},{1200,0}}; w1.attenuation_dB = 15; w1.thickness_mm = 240;
            w2.wallId = "WALL_F" + std::to_string(f) + "_EXT2"; w2.points = {{1200,0},{1200,800}}; w2.attenuation_dB = 15; w2.thickness_mm = 240;
            w3.wallId = "WALL_F" + std::to_string(f) + "_EXT3"; w3.points = {{1200,800},{0,800}}; w3.attenuation_dB = 15; w3.thickness_mm = 240;
            w4.wallId = "WALL_F" + std::to_string(f) + "_EXT4"; w4.points = {{0,800},{0,0}}; w4.attenuation_dB = 15; w4.thickness_mm = 240;
            // 中庭
            zf::Wall w5, w6, w7, w8;
            w5.wallId = "WALL_F" + std::to_string(f) + "_AT1"; w5.points = {{400,200},{800,200}}; w5.attenuation_dB = 5; w5.thickness_mm = 100;
            w6.wallId = "WALL_F" + std::to_string(f) + "_AT2"; w6.points = {{800,200},{800,600}}; w6.attenuation_dB = 5; w6.thickness_mm = 100;
            w7.wallId = "WALL_F" + std::to_string(f) + "_AT3"; w7.points = {{800,600},{400,600}}; w7.attenuation_dB = 5; w7.thickness_mm = 100;
            w8.wallId = "WALL_F" + std::to_string(f) + "_AT4"; w8.points = {{400,600},{400,200}}; w8.attenuation_dB = 5; w8.thickness_mm = 100;
            floor.walls = {w1, w2, w3, w4, w5, w6, w7, w8};
            proj.floors.push_back(floor);
        }
    }
    else if (templateName == "酒店客房层") {
        for (int f = 1; f <= 2; ++f) {
            zf::Floor floor;
            floor.floorId = "F" + std::to_string(f);
            floor.floorName = std::to_string(f) + "F 客房层";
            // 外墙
            zf::Wall w1, w2, w3, w4;
            w1.wallId = "WALL_F" + std::to_string(f) + "_EXT1"; w1.points = {{0,0},{1000,0}}; w1.attenuation_dB = 15; w1.thickness_mm = 240;
            w2.wallId = "WALL_F" + std::to_string(f) + "_EXT2"; w2.points = {{1000,0},{1000,500}}; w2.attenuation_dB = 15; w2.thickness_mm = 240;
            w3.wallId = "WALL_F" + std::to_string(f) + "_EXT3"; w3.points = {{1000,500},{0,500}}; w3.attenuation_dB = 15; w3.thickness_mm = 240;
            w4.wallId = "WALL_F" + std::to_string(f) + "_EXT4"; w4.points = {{0,500},{0,0}}; w4.attenuation_dB = 15; w4.thickness_mm = 240;
            // 走廊两侧客房隔墙
            std::vector<zf::Wall> walls = {w1, w2, w3, w4};
            for (int i = 1; i <= 5; ++i) {
                double x = i * 180;
                zf::Wall wa, wb;
                wa.wallId = "WALL_F" + std::to_string(f) + "_R" + std::to_string(i) + "A";
                wa.points = {{x,0},{x,200}}; wa.attenuation_dB = 10; wa.thickness_mm = 120;
                wb.wallId = "WALL_F" + std::to_string(f) + "_R" + std::to_string(i) + "B";
                wb.points = {{x,300},{x,500}}; wb.attenuation_dB = 10; wb.thickness_mm = 120;
                walls.push_back(wa);
                walls.push_back(wb);
            }
            // 走廊边界
            zf::Wall wc, wd;
            wc.wallId = "WALL_F" + std::to_string(f) + "_COR1"; wc.points = {{0,200},{1000,200}}; wc.attenuation_dB = 8; wc.thickness_mm = 100;
            wd.wallId = "WALL_F" + std::to_string(f) + "_COR2"; wd.points = {{0,300},{1000,300}}; wd.attenuation_dB = 8; wd.thickness_mm = 100;
            walls.push_back(wc);
            walls.push_back(wd);
            floor.walls = walls;
            proj.floors.push_back(floor);
        }
    }
    return proj;
}

void MainWindow::onAutoPlace()
{
    if (!m_project || m_project.floors.empty()) {
        QMessageBox::warning(this, "无法布放", "请先创建楼层");
        return;
    }

    // 参数对话框
    bool ok = false;
    double radius = QInputDialog::getDouble(this, "自动布放参数",
        "天线覆盖半径（米）:\n（建议: 室内5-10米，半径越小天线越密）",
        8.0, 3.0, 30.0, 1, &ok);
    if (!ok) return;

    // 发出修改前信号（用于Undo）
    emit m_canvas->projectAboutToChange();

    // 执行自动布放
    zf::AutoPlaceParams params;
    params.coverageRadius_m = radius;
    zf::AutoPlacer placer;
    auto result = placer.place(m_project.floors[m_currentFloorIndex], params);

    if (result.success) {
        // 发出修改后信号
        emit m_canvas->projectChanged("自动布放");
        m_canvas->refresh();
        m_propertyPanel->clear();
        QMessageBox::information(this, "自动布放完成",
            QString("当前楼层自动布放完成:\n\n"
                    "天线数量: %1 个\n"
                    "功分器数量: %2 个\n"
                    "信源数量: %3 个\n"
                    "线缆连接: %4 条\n\n"
                    "覆盖半径: %5 米")
                .arg(result.antennaCount)
                .arg(result.splitterCount)
                .arg(result.sourceCount)
                .arg(result.connectionCount)
                .arg(radius));
        statusBar()->showMessage(QString("自动布放完成: %1个天线, %2个功分器")
            .arg(result.antennaCount).arg(result.splitterCount));
    } else {
        QMessageBox::warning(this, "自动布放失败", QString::fromStdString(result.message));
    }
}
