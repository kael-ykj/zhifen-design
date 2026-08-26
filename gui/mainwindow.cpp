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
#include "io/drawing_exporter.h"
#include "io/project_io.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
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

    m_canvas = new CanvasWidget(this);
    m_canvas->setProject(&m_project);
    setCentralWidget(m_canvas);

    statusBar()->showMessage("就绪 | 当前模式: 草图模式 | 提示: 左键选择/放置, 中键平移, 滚轮缩放, Delete删除");

    connect(m_canvas, &CanvasWidget::deviceSelected, this, &MainWindow::onDeviceSelected);
    connect(m_canvas, &CanvasWidget::statusMessage, this, &MainWindow::onStatusMessage);
    connect(m_canvas, &CanvasWidget::deviceDeleted, this, &MainWindow::onDeviceDeleted);
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
    connect(m_actUndo, &QAction::triggered, this, [this](){ statusBar()->showMessage("撤销功能开发中"); });

    m_actRedo = new QAction("重做", this);
    m_actRedo->setShortcut(QKeySequence::Redo);
    m_actRedo->setEnabled(false);
    connect(m_actRedo, &QAction::triggered, this, [this](){ statusBar()->showMessage("重做功能开发中"); });

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

    m_actGenerateReport = new QAction("生成报告", this);
    connect(m_actGenerateReport, &QAction::triggered, this, &MainWindow::onGenerateReport);

    m_actExportMaterial = new QAction("材料表导出", this);
    connect(m_actExportMaterial, &QAction::triggered, this, &MainWindow::onExportMaterialList);

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
    fileMenu->addAction(m_actSave);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actExport);
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
    calcMenu->addAction(m_actGenerateReport);
    calcMenu->addSeparator();
    calcMenu->addAction(m_actExportMaterial);

    QMenu* helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction(m_actAbout);
}

void MainWindow::createToolBars()
{
    QToolBar* fileTool = addToolBar("文件");
    fileTool->addAction(m_actNew);
    fileTool->addAction(m_actOpen);
    fileTool->addAction(m_actSave);

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
    calcTool->addSeparator();
    calcTool->addAction(m_actExportMaterial);
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
    m_project.floors.clear();
    zf::Floor floor;
    floor.floorId = "F1";
    floor.floorName = "1F";
    m_project.floors.push_back(floor);
    m_canvas->clearHeatmap();
    m_heatmapVisible = false;
    m_actHeatmap->setChecked(false);
    m_canvas->refresh();
    m_propertyPanel->clear();
    statusBar()->showMessage("新建工程完成");
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
        statusBar()->showMessage("工程已加载: " + path);
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
        statusBar()->showMessage("当前工具: 绘制墙体 (开发中)");
    } else if (m_actToolCable->isChecked()) {
        m_canvas->setCurrentTool("cable");
        statusBar()->showMessage("当前工具: 绘制线缆 (开发中)");
    }
}

void MainWindow::onRunLinkCalc()
{
    if (m_project.floors.empty() || m_project.floors[0].devices.empty()) {
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
    if (m_project.floors.empty() || m_project.floors[0].devices.empty()) {
        QMessageBox::warning(this, "覆盖仿真", "请先放置器件");
        return;
    }
    // 找到第一个天线作为发射点
    zf::Point2D txPos{0, 0};
    bool found = false;
    for (const auto& dev : m_project.floors[0].devices) {
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
        txPos = m_project.floors[0].devices[0].position;
    }

    zf::PropagationEngine engine;
    engine.setModeManager(m_modeLayer.modeManager.get());
    zf::SimulationConfig cfg;
    cfg.gridResolution_m = 1.0;
    cfg.maxDistance_m = 80.0;
    engine.setConfig(cfg);

    zf::HeatmapData heatmap;
    int result = engine.generateHeatmap(&m_project.floors[0], txPos, heatmap);
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
    if (m_project.floors.empty() || m_project.floors[0].devices.empty()) {
        QMessageBox::warning(this, "系统图", "请先放置器件");
        return;
    }
    zf::SystemDiagramEngine engine;
    engine.setModeManager(m_modeLayer.modeManager.get());
    zf::SystemDiagram diagram;
    int result = engine.generateFromFloor(&m_project.floors[0], &m_project, diagram);
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
        QString msg = QString("工程造价概算完成!\n\n"
                              "器件总数: %1 个\n"
                              "线缆总长: %2 米\n\n"
                              "材料费:   %3 元\n"
                              "线缆费:   %4 元\n"
                              "人工费:   %5 元\n"
                              "其他费:   %6 元\n"
                              "小计:     %7 元\n"
                              "税金(9%%): %8 元\n"
                              "========================\n"
                              "含税总价: %9 元")
            .arg(summary.deviceCount)
            .arg(summary.cableLength_m, 0, 'f', 1)
            .arg(summary.materialCost, 0, 'f', 2)
            .arg(summary.cableCost, 0, 'f', 2)
            .arg(summary.laborCost, 0, 'f', 2)
            .arg(summary.otherCost, 0, 'f', 2)
            .arg(summary.subtotal, 0, 'f', 2)
            .arg(summary.tax, 0, 'f', 2)
            .arg(summary.total, 0, 'f', 2);
        QMessageBox::information(this, "造价概算", msg);
        statusBar()->showMessage(QString("造价概算完成，含税总价: %1 元").arg(summary.total, 0, 'f', 2));
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
    int result = exporter.exportFloorPlanDxf(path.toStdString(), &m_project.floors[0]);
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
