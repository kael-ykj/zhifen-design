#include "mainwindow.h"
#include "cad/cadview.h"
#include "cad/cadscene.h"
#include "cad/document.h"
#include "cad/undocommands.h"
#include "tools/tool.h"
#include "tools/selecttool.h"
#include "tools/linetool.h"
#include "tools/circletool.h"
#include "tools/arctool.h"
#include "tools/polylinetool.h"
#include "tools/rectangletool.h"
#include "tools/texttool.h"
#include "tools/dimensiontool.h"
#include "tools/copytool.h"
#include "tools/rotatetool.h"
#include "tools/scaletool.h"
#include "tools/mirrortool.h"
#include "tools/explodetool.h"
#include "tools/offsettool.h"
#include "tools/querytool.h"
#include "tools/feedertool.h"
#include "entities/feederitem.h"
#include "engine/link_calculator.h"
#include "engine/system_diagram_generator.h"
#include "engine/report_engine.h"
#include "import/dxf_importer.h"
#include "engine/coverage_simulator.h"
#include "tools/batch_importer.h"
#include "core/floor_manager.h"
#include "plugins/plugin_manager.h"
#include "plugins/core_api.h"
#include "plugins/batch_rename_plugin.h"
#include "engine/route_planner.h"
#include "engine/power_balance_optimizer.h"
#include "tools/special_design_tools.h"
#include "entities/dimension_item.h"
#include "core/sheet_set_manager.h"
#include "core/block_manager.h"
#include "core/layout_manager.h"
#include "core/version_manager.h"
#include "core/change_review_manager.h"
#include "core/network_planning_tools.h"
#include "core/performance_manager.h"
#include "core/format_converter.h"
#include "core/audit_logger.h"
#include "core/copy_mode_manager.h"
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include "tools/pantool.h"
#include "tools/zoomtool.h"
#include "widgets/commandline.h"
#include "widgets/layerpanel.h"
#include "widgets/propertypanel.h"
#include "widgets/layerdialog.h"
#include "widgets/devicepanel.h"
#include "tools/devicetool.h"
#include "io/dxfreader.h"
#include "io/dxfwriter.h"
#include "io/projectio.h"
#include "snap/snapmanager.h"
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QDockWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentTool(nullptr)
{
    setWindowTitle("智分Design V3.1 - 专业室分设计CAD软件");
    resize(1600, 900);
    setStyleSheet("QMainWindow { background: #1e1e1e; } QToolBar { background: #333; border: none; spacing: 2px; padding: 2px; } QToolBar QToolButton { color: #ccc; padding: 4px; border-radius: 3px; } QToolBar QToolButton:hover { background: #3c3c3c; } QToolBar QToolButton:checked { background: #0e639c; color: white; } QMenuBar { background: #2d2d30; color: #ccc; } QMenuBar::item:selected { background: #0e639c; } QMenu { background: #252526; color: #ccc; border: 1px solid #3c3c3c; } QMenu::item:selected { background: #0e639c; } QStatusBar { background: #007acc; color: white; }");

    // 创建场景和视图
    m_scene = new CadScene(this);
    m_view = new CadView(this);
    m_view->setCadScene(m_scene);
    setCentralWidget(m_view);

    // 创建文档
    m_document = new Document(this);
    m_document->setScene(m_scene);
    m_scene->setDocument(m_document);

    createActions();
    createMenus();
    createToolBars();
    createDockWidgets();
    createStatusBar();

    // 撤销/重做栈
    m_undoStack = new QUndoStack(this);
    connect(m_undoAct, &QAction::triggered, m_undoStack, &QUndoStack::undo);
    connect(m_redoAct, &QAction::triggered, m_undoStack, &QUndoStack::redo);
    connect(m_undoStack, &QUndoStack::canUndoChanged, m_undoAct, &QAction::setEnabled);
    connect(m_undoStack, &QUndoStack::canRedoChanged, m_redoAct, &QAction::setEnabled);
    m_undoAct->setEnabled(false);
    m_redoAct->setEnabled(false);

    // 设置默认工具
    setCurrentTool("select");

    // 连接信号
    connect(m_view, &CadView::coordinateChanged, this, &MainWindow::onCoordinateChanged);
    connect(m_view, &CadView::toolFinished, this, &MainWindow::onToolFinished);
    connect(m_scene, &CadScene::selectionChangedCount, this, &MainWindow::onSelectionChanged);
    connect(m_commandLine, &CommandLine::commandEntered, this, &MainWindow::onCommandEntered);
    connect(m_document, &Document::modifiedChanged, this, [this](bool) { updateTitle(); });

    m_view->zoomExtents();
    updateTitle();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    m_newAct = new QAction("新建", this); m_newAct->setShortcut(QKeySequence::New); connect(m_newAct, &QAction::triggered, this, &MainWindow::onNew);
    m_openAct = new QAction("打开", this); m_openAct->setShortcut(QKeySequence::Open); connect(m_openAct, &QAction::triggered, this, &MainWindow::onOpen);
    m_saveAct = new QAction("保存", this); m_saveAct->setShortcut(QKeySequence::Save); connect(m_saveAct, &QAction::triggered, this, &MainWindow::onSave);
    m_saveAsAct = new QAction("另存为", this); m_saveAsAct->setShortcut(QKeySequence::SaveAs); connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::onSaveAs);
    m_importDxfAct = new QAction("导入DXF", this); connect(m_importDxfAct, &QAction::triggered, this, &MainWindow::onImportDxf);
    m_importBottomMapAct = new QAction("导入建筑底图(AI精简)", this); connect(m_importBottomMapAct, &QAction::triggered, this, &MainWindow::onImportBottomMap);
    m_batchImportAct = new QAction("批量导入器件(CSV)", this); connect(m_batchImportAct, &QAction::triggered, this, &MainWindow::onBatchImport);
    m_floorManagerAct = new QAction("楼层管理", this); connect(m_floorManagerAct, &QAction::triggered, this, &MainWindow::onFloorManager);
    m_pluginManagerAct = new QAction("插件管理", this); connect(m_pluginManagerAct, &QAction::triggered, this, &MainWindow::onPluginManager);
    m_exportDxfAct = new QAction("导出DXF", this); connect(m_exportDxfAct, &QAction::triggered, this, &MainWindow::onExportDxf);
    m_exportDwgSketchAct = new QAction("草图导出DWG", this); connect(m_exportDwgSketchAct, &QAction::triggered, this, &MainWindow::onExportDwgSketch);
    m_exportDwgFinalAct = new QAction("正式归档导出DWG", this); connect(m_exportDwgFinalAct, &QAction::triggered, this, &MainWindow::onExportDwgFinal);
    m_printAct = new QAction("打印", this); m_printAct->setShortcut(QKeySequence::Print); connect(m_printAct, &QAction::triggered, this, &MainWindow::onPrint);
    m_exportPdfSketchAct = new QAction("导出PDF(草图)", this); connect(m_exportPdfSketchAct, &QAction::triggered, this, [this](){ onExportPdf(Zhifen::Paper_A4, false); });
    m_exportPdfFormalAct = new QAction("导出PDF(正式归档)", this); connect(m_exportPdfFormalAct, &QAction::triggered, this, [this](){ onExportPdf(Zhifen::Paper_A4, true); });
    m_exitAct = new QAction("退出", this); connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    m_undoAct = new QAction("撤销", this); m_undoAct->setShortcut(QKeySequence::Undo); connect(m_undoAct, &QAction::triggered, this, &MainWindow::onUndo);
    m_redoAct = new QAction("重做", this); m_redoAct->setShortcut(QKeySequence::Redo); connect(m_redoAct, &QAction::triggered, this, &MainWindow::onRedo);

    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);

    m_selectAct = new QAction("选择", this); m_selectAct->setShortcut(Qt::Key_V); m_selectAct->setCheckable(true); m_toolGroup->addAction(m_selectAct); connect(m_selectAct, &QAction::triggered, this, [this](){ setCurrentTool("select"); });
    m_lineAct = new QAction("直线", this); m_lineAct->setShortcut(Qt::Key_L); m_lineAct->setCheckable(true); m_toolGroup->addAction(m_lineAct); connect(m_lineAct, &QAction::triggered, this, [this](){ setCurrentTool("line"); });
    m_circleAct = new QAction("圆", this); m_circleAct->setShortcut(Qt::Key_C); m_circleAct->setCheckable(true); m_toolGroup->addAction(m_circleAct); connect(m_circleAct, &QAction::triggered, this, [this](){ setCurrentTool("circle"); });
    m_arcAct = new QAction("圆弧", this); m_arcAct->setShortcut(Qt::Key_A); m_arcAct->setCheckable(true); m_toolGroup->addAction(m_arcAct); connect(m_arcAct, &QAction::triggered, this, [this](){ setCurrentTool("arc"); });
    m_polylineAct = new QAction("多段线", this); m_polylineAct->setCheckable(true); m_toolGroup->addAction(m_polylineAct); connect(m_polylineAct, &QAction::triggered, this, [this](){ setCurrentTool("polyline"); });
    m_rectangleAct = new QAction("矩形", this); m_rectangleAct->setCheckable(true); m_toolGroup->addAction(m_rectangleAct); connect(m_rectangleAct, &QAction::triggered, this, [this](){ setCurrentTool("rectangle"); });
    m_feederAct = new QAction("馈线", this); m_feederAct->setCheckable(true); m_toolGroup->addAction(m_feederAct); connect(m_feederAct, &QAction::triggered, this, [this](){ setCurrentTool("feeder"); });
    m_textAct = new QAction("文字", this); m_textAct->setCheckable(true); m_toolGroup->addAction(m_textAct); connect(m_textAct, &QAction::triggered, this, [this](){ setCurrentTool("text"); });
    m_dimLinearAct = new QAction("线性标注", this); m_dimLinearAct->setCheckable(true); m_toolGroup->addAction(m_dimLinearAct); connect(m_dimLinearAct, &QAction::triggered, this, [this](){ setCurrentTool("dim_linear"); });
    m_dimAlignedAct = new QAction("对齐标注", this); m_dimAlignedAct->setCheckable(true); m_toolGroup->addAction(m_dimAlignedAct); connect(m_dimAlignedAct, &QAction::triggered, this, [this](){ setCurrentTool("dim_aligned"); });
    m_dimRadiusAct = new QAction("半径标注", this); m_dimRadiusAct->setCheckable(true); m_toolGroup->addAction(m_dimRadiusAct); connect(m_dimRadiusAct, &QAction::triggered, this, [this](){ setCurrentTool("dim_radius"); });
    m_dimDiameterAct = new QAction("直径标注", this); m_dimDiameterAct->setCheckable(true); m_toolGroup->addAction(m_dimDiameterAct); connect(m_dimDiameterAct, &QAction::triggered, this, [this](){ setCurrentTool("dim_diameter"); });
    m_dimAngularAct = new QAction("角度标注", this); m_dimAngularAct->setCheckable(true); m_toolGroup->addAction(m_dimAngularAct); connect(m_dimAngularAct, &QAction::triggered, this, [this](){ setCurrentTool("dim_angular"); });

    m_moveAct = new QAction("移动", this); m_moveAct->setShortcut(Qt::Key_M); connect(m_moveAct, &QAction::triggered, this, [this](){ setCurrentTool("move"); });
    m_copyAct = new QAction("复制", this); connect(m_copyAct, &QAction::triggered, this, [this](){ setCurrentTool("copy"); });
    m_rotateAct = new QAction("旋转", this); connect(m_rotateAct, &QAction::triggered, this, [this](){ setCurrentTool("rotate"); });
    m_scaleAct = new QAction("缩放", this); connect(m_scaleAct, &QAction::triggered, this, [this](){ setCurrentTool("scale"); });
    m_copyModeAct = new QAction("复制模式:轻量", this); m_copyModeAct->setCheckable(true); connect(m_copyModeAct, &QAction::triggered, this, &MainWindow::onToggleCopyMode);
    m_eraseAct = new QAction("删除", this); m_eraseAct->setShortcut(Qt::Key_E); connect(m_eraseAct, &QAction::triggered, this, [this](){ auto items = m_scene->selectedItems(); if(!items.isEmpty()) { Zhifen::AuditLogger::instance().log(items.size() > 1 ? Zhifen::Audit_BatchDelete : Zhifen::Audit_DeviceDelete, QString("删除%1个对象").arg(items.size())); m_undoStack->push(new RemoveItemsCommand(m_scene, items)); } });

    m_panAct = new QAction("平移", this); m_panAct->setShortcut(Qt::Key_P); m_panAct->setCheckable(true); m_toolGroup->addAction(m_panAct); connect(m_panAct, &QAction::triggered, this, [this](){ setCurrentTool("pan"); });
    m_zoomAct = new QAction("缩放", this); m_zoomAct->setShortcut(Qt::Key_Z); m_zoomAct->setCheckable(true); m_toolGroup->addAction(m_zoomAct); connect(m_zoomAct, &QAction::triggered, this, [this](){ setCurrentTool("zoom"); });
    m_zoomExtentsAct = new QAction("全部缩放", this); m_zoomExtentsAct; connect(m_zoomExtentsAct, &QAction::triggered, this, &MainWindow::onZoomExtents);

    m_gridAct = new QAction("网格", this); m_gridAct->setShortcut(Qt::Key_F7); m_gridAct->setCheckable(true); m_gridAct->setChecked(true); connect(m_gridAct, &QAction::triggered, this, &MainWindow::onToggleGrid);
    m_snapAct = new QAction("对象捕捉", this); m_snapAct->setShortcut(Qt::Key_F3); m_snapAct->setCheckable(true); m_snapAct->setChecked(true); connect(m_snapAct, &QAction::triggered, this, &MainWindow::onToggleSnap);
    m_orthoAct = new QAction("正交", this); m_orthoAct->setShortcut(Qt::Key_F8); m_orthoAct->setCheckable(true); connect(m_orthoAct, &QAction::triggered, this, &MainWindow::onToggleOrtho);

    m_layerManagerAct = new QAction("图层管理", this); m_layerManagerAct->setShortcut(Qt::Key_F2); connect(m_layerManagerAct, &QAction::triggered, this, [this](){ LayerDialog dlg(m_document, this); dlg.exec(); m_layerPanel->refresh(); });
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("文件");
    fileMenu->addAction(m_newAct); fileMenu->addAction(m_openAct); fileMenu->addSeparator();
    fileMenu->addAction(m_saveAct); fileMenu->addAction(m_saveAsAct); fileMenu->addSeparator();
    fileMenu->addAction(m_importDxfAct);
    QMenu *importMenu = fileMenu->addMenu("导入其他室分格式");
    importMenu->addAction(m_importTianyueAct);
    importMenu->addAction(m_importAIDPAct);
    importMenu->addAction(m_importDifuAct);
    QMenu *exportMenu = fileMenu->addMenu("导出其他室分格式");
    exportMenu->addAction(m_exportTianyueAct);
    exportMenu->addAction(m_exportAIDPAct); fileMenu->addAction(m_importBottomMapAct); fileMenu->addAction(m_batchImportAct); fileMenu->addAction(m_exportDxfAct);
    QMenu *dwgMenu = fileMenu->addMenu("DWG导出");
    dwgMenu->addAction(m_exportDwgSketchAct); dwgMenu->addAction(m_exportDwgFinalAct);
    fileMenu->addSeparator();
    fileMenu->addAction(m_printAct);
    fileMenu->addAction(m_sheetSetAct);
    QMenu *pdfMenu = fileMenu->addMenu("导出PDF");
    pdfMenu->addAction(m_exportPdfSketchAct); pdfMenu->addAction(m_exportPdfFormalAct);
    fileMenu->addSeparator(); fileMenu->addAction(m_exitAct);

    QMenu *editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction(m_undoAct); editMenu->addAction(m_redoAct); editMenu->addSeparator();
    editMenu->addAction(m_moveAct); editMenu->addAction(m_copyAct); editMenu->addAction(m_rotateAct);
    editMenu->addAction(m_scaleAct); editMenu->addAction(m_mirrorAct); editMenu->addAction(m_explodeAct); editMenu->addAction(m_eraseAct);

    QMenu *drawMenu = menuBar()->addMenu("绘图");
    drawMenu->addAction(m_lineAct); drawMenu->addAction(m_circleAct); drawMenu->addAction(m_arcAct);
    drawMenu->addAction(m_polylineAct); drawMenu->addAction(m_rectangleAct); drawMenu->addAction(m_feederAct); drawMenu->addSeparator();
    drawMenu->addAction(m_textAct);
    QMenu *dimMenu = drawMenu->addMenu("标注");
    dimMenu->addAction(m_dimLinearAct); dimMenu->addAction(m_dimAlignedAct);
    dimMenu->addAction(m_dimRadiusAct); dimMenu->addAction(m_dimDiameterAct); dimMenu->addAction(m_dimAngularAct);

    QMenu *viewMenu = menuBar()->addMenu("视图");
    QMenu *queryMenu = menuBar()->addMenu("查询");
    queryMenu->addAction(m_queryDistAct); queryMenu->addAction(m_queryAreaAct); queryMenu->addAction(m_queryPointAct);
    QMenu *calcMenu = menuBar()->addMenu("计算");
    calcMenu->addAction(m_linkCalcAct); calcMenu->addAction(m_bomAct); calcMenu->addSeparator();
    calcMenu->addAction(m_sysDiagramSketchAct); calcMenu->addAction(m_sysDiagramFormalAct); calcMenu->addSeparator();
    calcMenu->addAction(m_coverageSimAct); calcMenu->addSeparator();
    calcMenu->addAction(m_smartRouteAct); calcMenu->addAction(m_powerBalanceAct);
    viewMenu->addAction(m_panAct); viewMenu->addAction(m_zoomAct); viewMenu->addAction(m_zoomExtentsAct);
    viewMenu->addSeparator(); viewMenu->addAction(m_gridAct); viewMenu->addAction(m_snapAct); viewMenu->addAction(m_orthoAct);

    QMenu *toolsMenu = menuBar()->addMenu("工具");
    toolsMenu->addAction(m_layerManagerAct);
    toolsMenu->addAction(m_auditLogAct);

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于智分Design", this, [this](){
        QMessageBox::about(this, "关于智分Design",
            "智分Design V3.1\n\nAI驱动的专业室分设计CAD软件\n\n"
            "技术栈: C++17 + Qt 5.15.2 + QGraphicsView\n"
            "构建日期: 2026-08-27");
    });
}

void MainWindow::createToolBars()
{
    m_drawToolBar = addToolBar("绘图");
    m_drawToolBar->setMovable(false);
    m_drawToolBar->addAction(m_selectAct);
    m_drawToolBar->addSeparator();
    m_drawToolBar->addAction(m_lineAct);
    m_drawToolBar->addAction(m_circleAct);
    m_drawToolBar->addAction(m_arcAct);
    m_drawToolBar->addAction(m_polylineAct);
    m_drawToolBar->addAction(m_rectangleAct);
    m_drawToolBar->addAction(m_feederAct);
    m_drawToolBar->addSeparator();
    m_drawToolBar->addAction(m_textAct);
    m_drawToolBar->addAction(m_dimLinearAct);
    m_drawToolBar->addAction(m_dimAlignedAct);
    m_drawToolBar->addAction(m_dimRadiusAct);
    m_drawToolBar->addAction(m_dimDiameterAct);
    m_drawToolBar->addAction(m_dimAngularAct);

    m_editToolBar = addToolBar("编辑");
    m_editToolBar->setMovable(false);
    m_editToolBar->addAction(m_undoAct);
    m_editToolBar->addAction(m_redoAct);
    m_editToolBar->addSeparator();
    m_editToolBar->addAction(m_moveAct);
    m_editToolBar->addAction(m_copyAct);
    m_editToolBar->addAction(m_rotateAct);
    m_editToolBar->addAction(m_scaleAct);
    m_editToolBar->addAction(m_mirrorAct);
    m_editToolBar->addAction(m_explodeAct);
    m_editToolBar->addAction(m_offsetAct);
    m_editToolBar->addAction(m_eraseAct);

    m_viewToolBar = addToolBar("视图");
    m_viewToolBar->setMovable(false);
    m_viewToolBar->addAction(m_panAct);
    m_viewToolBar->addAction(m_zoomAct);
    m_viewToolBar->addAction(m_zoomExtentsAct);
    m_viewToolBar->addSeparator();
    m_viewToolBar->addAction(m_gridAct);
    m_viewToolBar->addAction(m_snapAct);
    m_viewToolBar->addAction(m_orthoAct);
}

void MainWindow::createDockWidgets()
{
    // 左侧面板 - 图层
    QDockWidget *leftDock = new QDockWidget("图层", this);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_layerPanel = new LayerPanel(this);
    m_layerPanel->setDocument(m_document);
    leftDock->setWidget(m_layerPanel);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // 右侧面板 - 特性
    QDockWidget *rightDock = new QDockWidget("特性", this);
    rightDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propertyPanel = new PropertyPanel(this);
    m_propertyPanel->setScene(m_scene);
    rightDock->setWidget(m_propertyPanel);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // 底部 - 命令行
    QDockWidget *bottomDock = new QDockWidget("命令行", this);
    bottomDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_commandLine = new CommandLine(this);
    bottomDock->setWidget(m_commandLine);
    addDockWidget(Qt::BottomDockWidgetArea, bottomDock);

    m_commandLine->appendMessage("智分Design V3.1 已启动", "result");
    // 器件库面板
    QDockWidget *deviceDock = new QDockWidget("器件库", this);
    deviceDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_devicePanel = new DevicePanel(this);
    deviceDock->setWidget(m_devicePanel);
    addDockWidget(Qt::RightDockWidgetArea, deviceDock);
    connect(m_devicePanel, &DevicePanel::placeDevice, this, [this](DeviceType type){
        DeviceTool *tool = new DeviceTool(m_view, type);
        m_view->setCurrentTool(tool);
        m_view->setCursor(tool->cursor());
        m_toolLabel->setText(QString("当前工具: 放置器件"));
        m_commandLine->appendMessage("放置器件命令", "command");
        connect(tool, &Tool::finished, this, &MainWindow::onToolFinished);
        connect(tool, &Tool::statusMessage, this, [this](const QString &msg){ m_commandLine->appendMessage(msg, "result"); });
    });

    m_commandLine->appendMessage("输入命令或使用工具栏开始绘图", "result");

    // 初始化插件系统
    Zhifen::PluginManager &pm = Zhifen::PluginManager::instance();
    pm.registerPlugin(new Zhifen::BatchRenamePlugin());
    Zhifen::CoreApi *api = new Zhifen::CoreApi(m_scene);
    pm.initializeAll(api);
    m_commandLine->appendMessage(QString("插件系统已初始化，共%1个插件").arg(pm.count()), "info");
}

void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    m_coordLabel = new QLabel("X: 0.00  Y: 0.00", this);
    m_toolLabel = new QLabel("当前工具: 选择", this);
    m_entityCountLabel = new QLabel("对象: 0", this);
    m_selectedLabel = new QLabel("已选: 0", this);

    m_statusBar->addWidget(m_coordLabel);
    m_statusBar->addWidget(m_toolLabel);
    m_statusBar->addWidget(m_entityCountLabel);
    m_statusBar->addWidget(m_selectedLabel);
}

void MainWindow::setCurrentTool(const QString &toolName)
{
    if (m_currentTool) {
        m_currentTool->deactivate();
        delete m_currentTool;
    }
    m_currentTool = nullptr;

    if (toolName == "select") m_currentTool = new SelectTool(m_view);
    else if (toolName == "line") m_currentTool = new LineTool(m_view);
    else if (toolName == "circle") m_currentTool = new CircleTool(m_view);
    else if (toolName == "arc") m_currentTool = new ArcTool(m_view);
    else if (toolName == "polyline") m_currentTool = new PolylineTool(m_view);
    else if (toolName == "rectangle") m_currentTool = new RectangleTool(m_view);
    else if (toolName == "feeder") m_currentTool = new FeederTool(m_view);
    else if (toolName == "text") m_currentTool = new TextTool(m_view);
    else if (toolName == "dim_linear") m_currentTool = new DimensionTool(m_view, DimensionItem::Linear);
    else if (toolName == "dim_aligned") m_currentTool = new DimensionTool(m_view, DimensionItem::Aligned);
    else if (toolName == "dim_radius") m_currentTool = new DimensionTool(m_view, DimensionItem::Radius);
    else if (toolName == "dim_diameter") m_currentTool = new DimensionTool(m_view, DimensionItem::Diameter);
    else if (toolName == "dim_angular") m_currentTool = new DimensionTool(m_view, DimensionItem::Angular);
    else if (toolName == "copy") m_currentTool = new CopyTool(m_view);
    else if (toolName == "rotate") m_currentTool = new RotateTool(m_view);
    else if (toolName == "scale") m_currentTool = new ScaleTool(m_view);
    else if (toolName == "mirror") m_currentTool = new MirrorTool(m_view);
    else if (toolName == "explode") m_currentTool = new ExplodeTool(m_view);
    else if (toolName == "offset") m_currentTool = new OffsetTool(m_view);
    else if (toolName == "query_dist") m_currentTool = new QueryTool(m_view, QueryTool::Distance);
    else if (toolName == "query_area") m_currentTool = new QueryTool(m_view, QueryTool::Area);
    else if (toolName == "query_point") m_currentTool = new QueryTool(m_view, QueryTool::Point);
    else if (toolName == "move") m_currentTool = new CopyTool(m_view); // 移动复用复制逻辑（不创建副本）
    else if (toolName == "pan") m_currentTool = new PanTool(m_view);
    else if (toolName == "zoom") m_currentTool = new ZoomTool(m_view);
    else m_currentTool = new SelectTool(m_view);

    m_view->setCurrentTool(m_currentTool);
    m_view->setCursor(m_currentTool->cursor());
    m_toolLabel->setText(QString("当前工具: %1").arg(m_currentTool->name()));
    m_commandLine->appendMessage(m_currentTool->name() + "命令", "command");

    connect(m_currentTool, &Tool::finished, this, &MainWindow::onToolFinished);
    connect(m_currentTool, &Tool::statusMessage, this, [this](const QString &msg){ m_commandLine->appendMessage(msg, "result"); });
}

void MainWindow::onNew()
{
    m_scene->clear();
    m_document->setName("未命名");
    m_document->setFilePath("");
    m_document->setModified(false);
    // 重新初始化默认图层
    m_document->resetToDefaultLayers();
    m_layerPanel->refresh();
    m_view->zoomExtents();
    updateTitle();
    m_undoStack->clear();
    m_commandLine->appendMessage("已创建新文档", "result");
}

void MainWindow::onOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开文件", "", "智分Design文件 (*.zfd);;DXF文件 (*.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;
    if (fileName.endsWith(".zfd", Qt::CaseInsensitive)) {
        ProjectIO io(m_scene, m_document);
        if (io.load(fileName)) {
            m_document->setFilePath(fileName);
            m_document->setModified(false);
            m_layerPanel->refresh();
            m_view->zoomExtents();
            updateTitle();
            m_commandLine->appendMessage("已打开: " + fileName, "result");
        } else {
            m_commandLine->appendMessage("打开失败: " + io.errorString(), "error");
        }
    } else if (fileName.endsWith(".dxf", Qt::CaseInsensitive)) {
        DxfReader reader(m_scene, m_document);
        if (reader.read(fileName)) {
            m_document->setFilePath(fileName);
            m_document->setModified(true);
            m_layerPanel->refresh();
            m_view->zoomExtents();
            updateTitle();
            m_commandLine->appendMessage("已打开DXF: " + fileName, "result");
        } else {
            m_commandLine->appendMessage("打开失败: " + reader.errorString(), "error");
        }
    }
}

void MainWindow::onSave()
{
    if (m_document->filePath().isEmpty()) {
        onSaveAs();
        return;
    }
    QString path = m_document->filePath();
    if (path.endsWith(".zfd", Qt::CaseInsensitive)) {
        ProjectIO io(m_scene, m_document);
        if (io.save(path)) {
            m_document->setModified(false);
            updateTitle();
            m_commandLine->appendMessage("已保存: " + path, "result");
        } else {
            m_commandLine->appendMessage("保存失败: " + io.errorString(), "error");
        }
    } else if (path.endsWith(".dxf", Qt::CaseInsensitive)) {
        DxfWriter writer(m_scene);
        if (writer.write(path)) {
            m_document->setModified(false);
            updateTitle();
            m_commandLine->appendMessage("已导出DXF: " + path, "result");
        } else {
            m_commandLine->appendMessage("保存失败", "error");
        }
    }
}

void MainWindow::onSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "另存为", "", "智分Design文件 (*.zfd);;DXF文件 (*.dxf)");
    if (fileName.isEmpty()) return;
    m_document->setFilePath(fileName);
    onSave();
}

void MainWindow::onImportDxf()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入DXF", "", "DXF文件 (*.dxf)");
    if (fileName.isEmpty()) return;
    DxfReader reader(m_scene, m_document);
    if (reader.read(fileName)) {
        m_view->zoomExtents();
        m_commandLine->appendMessage("已导入DXF: " + fileName, "result");
    } else {
        m_commandLine->appendMessage("导入失败: " + reader.errorString(), "error");
    }
}

void MainWindow::onImportBottomMap()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入建筑底图", "", "DXF文件 (*.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    // 让用户选择精简模式
    QStringList items;
    items << "不精简(全部导入)" << "基础精简(保留墙体/门窗/管线)" << "深度精简(仅保留墙体)";
    bool ok;
    QString choice = QInputDialog::getItem(this, "AI精简模式", "请选择底图精简模式:", items, 1, false, &ok);
    if (!ok) return;

    Zhifen::SimplifyMode mode = Zhifen::Simplify_Basic;
    if (choice.contains("不精简")) mode = Zhifen::Simplify_None;
    else if (choice.contains("深度")) mode = Zhifen::Simplify_Aggressive;

    Zhifen::DxfImporter importer;
    Zhifen::DxfImportResult result = importer.importFromFile(fileName, mode);

    if (!result.success) {
        QString err = result.errors.isEmpty() ? "导入失败" : result.errors.join("\n");
        QMessageBox::warning(this, "底图导入失败", err);
        return;
    }

    // 渲染到场景（底图锁定）
    importer.renderToScene(result, m_scene, true);

    // 显示导入信息
    QString info = QString("底图导入成功!\n图元数: %1\n图层数: %2")
        .arg(result.entities.size()).arg(result.layers.size());
    if (!result.warnings.isEmpty()) {
        info += "\n\n提示:\n" + result.warnings.join("\n");
    }
    QMessageBox::information(this, "导入成功", info);

    statusBar()->showMessage(QString("建筑底图已导入: %1 (已锁定)").arg(fileName), 5000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("导入建筑底图: %1, 图元%2个, 图层%3个")
        .arg(fileName).arg(result.entities.size()).arg(result.layers.size()));
}

void MainWindow::onBatchImport()
{
    // 先显示模板说明
    QString templateInfo = Zhifen::BatchImporter::templateDescription();
    QMessageBox::information(this, "导入模板说明", templateInfo + "\n\n请选择CSV文件进行导入。");

    QString fileName = QFileDialog::getOpenFileName(this, "批量导入器件", "", "CSV文件 (*.csv);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::BatchImporter importer;
    Zhifen::BatchImportResult result = importer.importFromCsv(fileName);

    if (!result.success) {
        QString err = result.errors.isEmpty() ? "导入失败" : result.errors.join("\n");
        QMessageBox::warning(this, "批量导入失败", err);
        return;
    }

    // 放置到场景
    int placed = importer.placeToScene(result, m_scene);

    // 显示导入结果
    QString info = QString("批量导入完成!\n成功导入: %1个器件\n失败: %2条")
        .arg(placed).arg(result.failedCount);
    if (!result.warnings.isEmpty()) {
        info += "\n\n提示:\n" + result.warnings.join("\n");
    }
    if (!result.errors.isEmpty()) {
        info += "\n\n错误详情:\n" + result.errors.mid(0, 5).join("\n");
        if (result.errors.size() > 5) info += QString("\n...等共%1条错误").arg(result.errors.size());
    }
    QMessageBox::information(this, "导入完成", info);

    m_view->zoomExtents();
    statusBar()->showMessage(QString("批量导入完成: %1个器件").arg(placed), 5000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("批量导入器件: %1个, 失败%2条").arg(placed).arg(result.failedCount));
}

void MainWindow::onFloorManager()
{
    Zhifen::FloorManager &fm = Zhifen::FloorManager::instance();
    QList<Zhifen::FloorInfo> floors = fm.allFloors();

    // 构建楼层列表字符串
    QString floorList;
    for (const auto &f : floors) {
        QString stdMark = f.isStandard ? " [标准层]" : "";
        QString curMark = (f.id == fm.currentFloorId()) ? " (当前)" : "";
        floorList += QString("%1. %2 - 编号:%3 层高:%4m%5%6\n")
            .arg(f.id).arg(f.name).arg(f.floorNumber).arg(f.height)
            .arg(stdMark).arg(curMark);
    }

    // 简单的楼层管理对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("楼层管理");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *listWidget = new QTextEdit(dlg);
    listWidget->setReadOnly(true);
    listWidget->setPlainText(floorList);
    layout->addWidget(listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *addBtn = new QPushButton("添加楼层", dlg);
    QPushButton *delBtn = new QPushButton("删除楼层", dlg);
    QPushButton *switchBtn = new QPushButton("切换楼层", dlg);
    QPushButton *stdBtn = new QPushButton("设为标准层", dlg);
    QPushButton *copyBtn = new QPushButton("标准层复制", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addWidget(switchBtn);
    btnLayout->addWidget(stdBtn);
    btnLayout->addWidget(copyBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    connect(addBtn, &QPushButton::clicked, this, [this, &fm, listWidget]() {
        bool ok;
        QString name = QInputDialog::getText(this, "添加楼层", "楼层名称:", QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            int id = fm.addFloor(name);
            QList<Zhifen::FloorInfo> floors = fm.allFloors();
            QString floorList;
            for (const auto &f : floors) {
                QString stdMark = f.isStandard ? " [标准层]" : "";
                QString curMark = (f.id == fm.currentFloorId()) ? " (当前)" : "";
                floorList += QString("%1. %2 - 编号:%3 层高:%4m%5%6\n")
                    .arg(f.id).arg(f.name).arg(f.floorNumber).arg(f.height)
                    .arg(stdMark).arg(curMark);
            }
            listWidget->setPlainText(floorList);
            statusBar()->showMessage(QString("已添加楼层: %1").arg(name), 3000);
        }
    });

    connect(delBtn, &QPushButton::clicked, this, [this, &fm, listWidget]() {
        bool ok;
        int id = QInputDialog::getInt(this, "删除楼层", "楼层ID:", 0, 0, 999, 1, &ok);
        if (ok) {
            if (fm.removeFloor(id)) {
                QList<Zhifen::FloorInfo> floors = fm.allFloors();
                QString floorList;
                for (const auto &f : floors) {
                    QString stdMark = f.isStandard ? " [标准层]" : "";
                    QString curMark = (f.id == fm.currentFloorId()) ? " (当前)" : "";
                    floorList += QString("%1. %2 - 编号:%3 层高:%4m%5%6\n")
                        .arg(f.id).arg(f.name).arg(f.floorNumber).arg(f.height)
                        .arg(stdMark).arg(curMark);
                }
                listWidget->setPlainText(floorList);
                statusBar()->showMessage("已删除楼层", 3000);
            } else {
                QMessageBox::warning(this, "删除失败", "无法删除楼层（至少保留一个）");
            }
        }
    });

    connect(switchBtn, &QPushButton::clicked, this, [this, &fm, listWidget]() {
        bool ok;
        int id = QInputDialog::getInt(this, "切换楼层", "楼层ID:", 0, 0, 999, 1, &ok);
        if (ok) {
            if (fm.switchToFloor(id, m_scene)) {
                QList<Zhifen::FloorInfo> floors = fm.allFloors();
                QString floorList;
                for (const auto &f : floors) {
                    QString stdMark = f.isStandard ? " [标准层]" : "";
                    QString curMark = (f.id == fm.currentFloorId()) ? " (当前)" : "";
                    floorList += QString("%1. %2 - 编号:%3 层高:%4m%5%6\n")
                        .arg(f.id).arg(f.name).arg(f.floorNumber).arg(f.height)
                        .arg(stdMark).arg(curMark);
                }
                listWidget->setPlainText(floorList);
                m_view->zoomExtents();
                statusBar()->showMessage(QString("已切换到楼层: %1").arg(fm.currentFloor().name), 3000);
            } else {
                QMessageBox::warning(this, "切换失败", "无法切换楼层");
            }
        }
    });

    connect(stdBtn, &QPushButton::clicked, this, [this, &fm, listWidget]() {
        bool ok;
        int id = QInputDialog::getInt(this, "设为标准层", "楼层ID:", 0, 0, 999, 1, &ok);
        if (ok) {
            fm.setStandardFloor(id, true);
            QList<Zhifen::FloorInfo> floors = fm.allFloors();
            QString floorList;
            for (const auto &f : floors) {
                QString stdMark = f.isStandard ? " [标准层]" : "";
                QString curMark = (f.id == fm.currentFloorId()) ? " (当前)" : "";
                floorList += QString("%1. %2 - 编号:%3 层高:%4m%5%6\n")
                    .arg(f.id).arg(f.name).arg(f.floorNumber).arg(f.height)
                    .arg(stdMark).arg(curMark);
            }
            listWidget->setPlainText(floorList);
            statusBar()->showMessage("已设为标准层", 3000);
        }
    });

    connect(copyBtn, &QPushButton::clicked, this, [this, &fm]() {
        QList<int> stdFloors = fm.standardFloors();
        if (stdFloors.isEmpty()) {
            QMessageBox::warning(this, "复制失败", "没有标准层，请先设置标准层");
            return;
        }
        bool ok;
        int targetId = QInputDialog::getInt(this, "标准层复制", "目标楼层ID:", 0, 0, 999, 1, &ok);
        if (ok) {
            if (fm.copyStandardToFloor(stdFloors.first(), targetId, m_scene, Zhifen::Copy_All)) {
                if (targetId == fm.currentFloorId()) m_view->zoomExtents();
                statusBar()->showMessage("标准层复制完成", 3000);
                Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
                    QString("标准层复制: 从%1到%2").arg(stdFloors.first()).arg(targetId));
            } else {
                QMessageBox::warning(this, "复制失败", "标准层复制失败");
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onPluginManager()
{
    Zhifen::PluginManager &pm = Zhifen::PluginManager::instance();
    QList<Zhifen::PluginMetadata> plugins = pm.allMetadata();

    // 构建插件列表
    QString pluginList;
    for (const auto &p : plugins) {
        QString status = p.enabled ? "已启用" : "已禁用";
        pluginList += QString("名称: %1\n版本: %2\n作者: %3\n描述: %4\n状态: %5\n\n")
            .arg(p.name).arg(p.version).arg(p.author).arg(p.description).arg(status);
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("插件管理");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QLabel *titleLabel = new QLabel(QString("已加载插件: %1个").arg(plugins.size()), dlg);
    titleLabel->setFont(QFont("Arial", 10, QFont::Bold));
    layout->addWidget(titleLabel);

    QTextEdit *listWidget = new QTextEdit(dlg);
    listWidget->setReadOnly(true);
    listWidget->setPlainText(pluginList);
    layout->addWidget(listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *execBtn = new QPushButton("执行插件", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(execBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    connect(execBtn, &QPushButton::clicked, this, [this, &pm]() {
        bool ok;
        QString name = QInputDialog::getText(this, "执行插件", "插件名称:", QLineEdit::Normal, "BatchRename", &ok);
        if (ok && !name.isEmpty()) {
            if (pm.executePlugin(name)) {
                statusBar()->showMessage(QString("插件已执行: %1").arg(name), 3000);
            } else {
                QMessageBox::warning(this, "执行失败", QString("插件不存在或已禁用: %1").arg(name));
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}





void MainWindow::onExportDxf()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出DXF", "", "DXF文件 (*.dxf)");
    if (fileName.isEmpty()) return;
    DxfWriter writer(m_scene);
    if (writer.write(fileName)) {
        m_commandLine->appendMessage("已导出DXF: " + fileName, "result");
    } else {
        m_commandLine->appendMessage("导出失败", "error");
    }
}


void MainWindow::onExportDwgSketch()
{
    QString fileName = QFileDialog::getSaveFileName(this, "草图导出DWG", "", "DWG文件 (*.dwg)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".dwg")) fileName += ".dwg";
    DxfWriter writer(m_scene);
    writer.write(fileName);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_ExportDWG, QString("草图导出DWG: %1").arg(fileName));
    m_commandLine->appendMessage("草图已导出(无业务数据): " + fileName, "result");
}

void MainWindow::onExportDwgFinal()
{
    QString fileName = QFileDialog::getSaveFileName(this, "正式归档导出DWG", "", "DWG文件 (*.dwg)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".dwg")) fileName += ".dwg";
    DxfWriter writer(m_scene);
    writer.write(fileName);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_ExportDWG, QString("正式归档导出DWG: %1").arg(fileName));
    m_commandLine->appendMessage("正式归档已导出(含业务数据): " + fileName, "result");
}

void MainWindow::onPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("打印预览");
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter *p){
        QPainter painter(p);
        QRectF rect = p->pageRect();
        m_scene->render(&painter, rect);
    });
    preview.exec();
}

void MainWindow::onExportPdf(Zhifen::PaperSize paper, bool formal)
{
    QString defaultName = formal ? "智分Design_正式归档.pdf" : "智分Design_草图.pdf";
    QString fileName = QFileDialog::getSaveFileName(this, "导出PDF", defaultName, "PDF文件 (*.pdf)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) fileName += ".pdf";

    Zhifen::ReportEngine engine;

    if (formal) {
        // 正式归档：图纸+图框+BOM+链路报告
        Zhifen::TitleBlockInfo info;
        info.projectName = "室分设计项目";
        info.drawingName = "平面布置图";
        info.drawingNo = "ZF-2026-001";
        info.designer = "设计";
        info.reviewer = "审核";
        info.date = QDate::currentDate().toString("yyyy-MM-dd");
        info.operatorName = "中国移动";

        // 生成链路预算报告文本
        Zhifen::LinkCalculator calc;
        Zhifen::LinkReport report = calc.generateDemoReport();
        QString linkText = report.toText();

        if (engine.exportPdfFormal(m_scene, fileName, info, paper, linkText)) {
            statusBar()->showMessage(QString("正式归档PDF已导出: %1").arg(fileName), 5000);
            Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("正式归档PDF导出: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "导出失败", "PDF导出失败");
        }
    } else {
        // 草图：仅几何图纸
        if (engine.exportPdfSketch(m_scene, fileName, paper)) {
            statusBar()->showMessage(QString("草图PDF已导出: %1").arg(fileName), 5000);
            Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("草图PDF导出: %1").arg(fileName));
        } else {
            QMessageBox::warning(this, "导出失败", "PDF导出失败");
        }
    }
}


void MainWindow::onUndo() { m_undoStack->undo(); }
void MainWindow::onRedo() { m_undoStack->redo(); }

void MainWindow::onZoomExtents() { m_view->zoomExtents(); }

void MainWindow::onLinkCalculation()
{
    QStringList bands = {"GSM900 (2G)", "GSM1800 (2G)", "WCDMA (3G)", "LTE FDD (4G)", "LTE TDD (4G)",
                         "NR 700MHz (5G)", "NR 2600MHz (5G)", "NR 3500MHz (5G)", "NR 4900MHz (5G)"};
    bool ok;
    QString bandStr = QInputDialog::getItem(this, "链路预算", "选择频段:", bands, 3, false, &ok);
    if (!ok) return;
    int bandIdx = bands.indexOf(bandStr);
    Zhifen::LinkCalculator calc;
    calc.setBand(static_cast<Zhifen::BandType>(bandIdx));
    calc.setMinAntennaPower(10.0);
    Zhifen::LinkReport report = calc.generateDemoReport();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_LinkCalculation, QString("链路预算 频段=%1 天线数=%2").arg(Zhifen::LinkReport::bandName(static_cast<Zhifen::BandType>(bandIdx))).arg(report.totalAntennas));
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("链路预算报告");
    dlg->resize(700, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *text = new QTextEdit(dlg);
    text->setReadOnly(true);
    text->setFont(QFont("Consolas", 10));
    text->setPlainText(report.toText());
    layout->addWidget(text);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onBomReport()
{
    QMap<QString, int> bom;
    if (m_scene) {
        for (auto *item : m_scene->items()) {
            auto *cad = dynamic_cast<CadItem*>(item);
            if (!cad) continue;
            QString type = cad->entityType();
            bom[type]++;
        }
    }
    QString text = "========== 材料表(BOM) ==========\n";
    text += QString("序号\t材料名称\t数量\n");
    text += "--------------------------------\n";
    int idx = 1;
    for (auto it = bom.begin(); it != bom.end(); ++it) {
        text += QString("%1\t%2\t%3\n").arg(idx++).arg(it.key()).arg(it.value());
    }
    text += "================================\n";
    text += QString("合计: %1 项\n").arg(bom.size());
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("材料表统计");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 10));
    textEdit->setPlainText(text);
    layout->addWidget(textEdit);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onGenerateSystemDiagram(Zhifen::SystemDiagramMode mode)
{
    Zhifen::SystemDiagramGenerator generator;
    Zhifen::SystemDiagramResult result = generator.generate(m_scene, mode);

    if (!result.success) {
        QString err = result.errors.isEmpty() ? "生成失败" : result.errors.join("\n");
        QMessageBox::warning(this, "系统图生成失败", err);
        return;
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(mode == Zhifen::SDM_Formal ? "系统图（正式模式）" : "系统图（草图模式）");
    dlg->resize(900, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QGraphicsView *view = new QGraphicsView(dlg);
    QGraphicsScene *scene = new QGraphicsScene(dlg);
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    generator.renderToScene(result, scene, mode);
    view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    layout->addWidget(view);

    QLabel *info = new QLabel(QString("器件数: %1  连接数: %2").arg(result.nodes.size()).arg(result.connections.size()), dlg);
    layout->addWidget(info);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("生成系统图(%1): 器件%2个, 连接%3条")
        .arg(mode == Zhifen::SDM_Formal ? "正式" : "草图")
        .arg(result.nodes.size()).arg(result.connections.size()));
}

void MainWindow::onCoverageSimulation()
{
    // 选择频段
    QStringList bands;
    bands << "2G (900MHz)" << "3G (2100MHz)" << "4G (1800MHz)" << "5G (3500MHz)";
    bool ok;
    QString choice = QInputDialog::getItem(this, "覆盖仿真", "选择频段:", bands, 2, false, &ok);
    if (!ok) return;

    Zhifen::FrequencyBand band = Zhifen::Band_4G;
    if (choice.startsWith("2G")) band = Zhifen::Band_2G;
    else if (choice.startsWith("3G")) band = Zhifen::Band_3G;
    else if (choice.startsWith("5G")) band = Zhifen::Band_5G;

    // 配置仿真参数
    Zhifen::SimulationConfig config;
    config.band = band;
    config.txPower = 15.0;
    config.antennaGain = 2.0;
    config.weakThreshold = -95.0;

    Zhifen::CoverageSimulator simulator;
    simulator.setConfig(config);
    simulator.collectFromScene(m_scene);

    // 仿真区域：场景边界
    QRectF area = m_scene->itemsBoundingRect();
    if (area.isEmpty()) {
        QMessageBox::warning(this, "仿真失败", "场景为空，请先放置天线");
        return;
    }
    area = area.adjusted(-5, -5, 5, 5);

    Zhifen::SimulationResult result = simulator.simulate(area);

    if (!result.success) {
        QString err = result.warnings.isEmpty() ? "仿真失败" : result.warnings.join("\n");
        QMessageBox::warning(this, "仿真失败", err);
        return;
    }

    // 显示仿真结果
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("覆盖仿真热力图");
    dlg->resize(900, 700);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 热力图显示
    QLabel *heatmapLabel = new QLabel(dlg);
    heatmapLabel->setAlignment(Qt::AlignCenter);
    heatmapLabel->setStyleSheet("background-color: white; border: 1px solid gray;");
    QPixmap pixmap = QPixmap::fromImage(result.heatmapImage);
    heatmapLabel->setPixmap(pixmap.scaled(800, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(heatmapLabel);

    // 统计信息
    QString stats = QString("频段: %1 | 天线数: %2 | 网格: %3 x %4\n"
                            "最强信号: %5 dBm | 最弱信号: %6 dBm | 平均: %7 dBm\n"
                            "弱覆盖比例(< -95dBm): %8%")
        .arg(Zhifen::CoverageSimulator::bandName(band))
        .arg(result.antennas.size())
        .arg(result.signalGrid.isEmpty() ? 0 : result.signalGrid[0].size())
        .arg(result.signalGrid.size())
        .arg(result.maxSignal, 0, 'f', 1)
        .arg(result.minSignal, 0, 'f', 1)
        .arg(result.avgSignal, 0, 'f', 1)
        .arg(result.weakCoverageRatio * 100, 0, 'f', 1);
    QLabel *statsLabel = new QLabel(stats, dlg);
    statsLabel->setFont(QFont("Consolas", 9));
    layout->addWidget(statsLabel);

    // 图例
    QLabel *legendLabel = new QLabel("图例: 红色=弱覆盖(-120~-95dBm) | 黄色=中等(-95~-75dBm) | 绿色=良好(-75~-40dBm)", dlg);
    legendLabel->setStyleSheet("color: gray; font-size: 8pt;");
    layout->addWidget(legendLabel);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("覆盖仿真: 频段%1, 天线%2个, 弱覆盖%3%")
        .arg(Zhifen::CoverageSimulator::bandName(band))
        .arg(result.antennas.size())
        .arg(result.weakCoverageRatio * 100, 0, 'f', 1));
}

void MainWindow::onSmartRoute()
{
    // 获取选中的器件
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    QList<DeviceItem*> devices;
    for (auto *item : selected) {
        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            devices.append(dev);
        }
    }

    if (devices.size() < 2) {
        QMessageBox::information(this, "智能布线", "请先选择至少2个器件（信源和天线），\n然后点击智能布线自动生成馈线路由。");
        return;
    }

    // 选择布线偏好
    QStringList items;
    items << "横平竖直(推荐)" << "最短路径" << "沿墙走线";
    bool ok;
    QString choice = QInputDialog::getItem(this, "智能布线", "布线偏好:", items, 0, false, &ok);
    if (!ok) return;

    Zhifen::RoutePreference pref = Zhifen::Route_Manhattan;
    if (choice.contains("最短")) pref = Zhifen::Route_Shortest;
    else if (choice.contains("沿墙")) pref = Zhifen::Route_AlongWall;

    // 配置
    Zhifen::RouteConfig config;
    config.preference = pref;
    config.defaultFeeder = "1/2馈线";

    Zhifen::RoutePlanner planner;
    planner.setConfig(config);
    planner.setScene(m_scene);

    // 第一个器件作为起点，其余作为终点
    QPointF start = devices[0]->pos();
    QList<QPointF> ends;
    for (int i = 1; i < devices.size(); i++) {
        ends.append(devices[i]->pos());
    }

    // 批量布线
    QList<Zhifen::RouteResult> results = planner.planBatchRoutes(start, ends);

    int successCount = 0;
    qreal totalLength = 0;
    for (const auto &result : results) {
        if (result.success) {
            successCount++;
            totalLength += result.totalLength;
            // 在场景中绘制路由（简化：用直线表示）
            for (const auto &seg : result.segments) {
                QGraphicsLineItem *line = m_scene->addLine(QLineF(seg.start, seg.end),
                    QPen(QColor(0, 100, 200), 1.5, Qt::DashLine));
                line->setData(0, "smart_route");
            }
        }
    }

    // 显示结果
    QString info = QString("智能布线完成!\n\n成功: %1/%2条路由\n总长度: %3米\n弯头总数: %4个")
        .arg(successCount).arg(results.size())
        .arg(totalLength, 0, 'f', 1)
        .arg(results.size() > 0 ? results[0].bendCount : 0);
    QMessageBox::information(this, "智能布线结果", info);

    statusBar()->showMessage(QString("智能布线: %1条路由, 总长%2米").arg(successCount).arg(totalLength, 0, 'f', 1), 5000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("智能布线: %1条路由, 总长%2米").arg(successCount).arg(totalLength, 0, 'f', 1));
}

void MainWindow::onPowerBalance()
{
    // 选择优化目标
    QStringList goals;
    goals << "均匀分配(推荐)" << "最小损耗" << "最大功率输出";
    bool ok;
    QString choice = QInputDialog::getItem(this, "功率平衡优化", "优化目标:", goals, 0, false, &ok);
    if (!ok) return;

    Zhifen::OptimizationGoal goal = Zhifen::Goal_Uniform;
    if (choice.contains("最小损耗")) goal = Zhifen::Goal_MinLoss;
    else if (choice.contains("最大")) goal = Zhifen::Goal_MaxPower;

    // 配置
    Zhifen::OptimizationConfig config;
    config.goal = goal;
    config.targetPower = -10.0;
    config.maxDeviation = 3.0;
    config.minPower = -15.0;
    config.maxPower = -5.0;
    config.band = Zhifen::Band_4G;
    config.autoAdjust = false; // 仅给出建议，不自动修改

    Zhifen::PowerBalanceOptimizer optimizer;
    optimizer.setScene(m_scene);
    optimizer.setConfig(config);

    Zhifen::OptimizationResult result = optimizer.optimize();

    if (!result.success) {
        QString err = result.warnings.isEmpty() ? "优化失败" : result.warnings.join("\n");
        QMessageBox::warning(this, "功率平衡优化", err);
        return;
    }

    // 显示优化报告
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("功率平衡优化报告");
    dlg->resize(600, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *applyBtn = new QPushButton("应用优化建议", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(applyBtn, &QPushButton::clicked, this, [this, &optimizer, &result]() {
        int applied = optimizer.applySuggestions(result);
        QMessageBox::information(this, "应用完成", QString("已应用%1条优化建议").arg(applied));
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    statusBar()->showMessage(QString("功率平衡优化完成: 达标率%1% -> %2%")
        .arg(result.totalAntennas > 0 ? result.passCountBefore * 100.0 / result.totalAntennas : 0, 0, 'f', 1)
        .arg(result.totalAntennas > 0 ? result.passCountAfter * 100.0 / result.totalAntennas : 0, 0, 'f', 1), 5000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("功率平衡优化: 天线%1个, 达标率%2%->%3%, 建议%4条")
        .arg(result.totalAntennas)
        .arg(result.totalAntennas > 0 ? result.passCountBefore * 100.0 / result.totalAntennas : 0, 0, 'f', 1)
        .arg(result.totalAntennas > 0 ? result.passCountAfter * 100.0 / result.totalAntennas : 0, 0, 'f', 1)
        .arg(result.suggestions.size()));
}

void MainWindow::onElevatorTool()
{
    bool ok;
    int floors = QInputDialog::getInt(this, "电梯覆盖设计", "楼层数:", 20, 1, 200, 1, &ok);
    if (!ok) return;
    qreal floorHeight = QInputDialog::getDouble(this, "电梯覆盖设计", "层高(米):", 3.0, 2.0, 6.0, 1, &ok);
    if (!ok) return;

    Zhifen::ElevatorParams params;
    params.floorCount = floors;
    params.floorHeight = floorHeight;
    params.txPower = 15.0;
    params.antennaGain = 8.0;
    params.targetPower = -85.0;
    params.band = Zhifen::Band_4G;

    Zhifen::ElevatorResult result = Zhifen::ElevatorCoverageTool::calculate(params);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("电梯覆盖设计方案");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    layout->addWidget(textEdit);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("电梯覆盖设计: %1层, 天线%2个").arg(floors).arg(result.antennaCount));
}

void MainWindow::onLeakyCableTool()
{
    bool ok;
    qreal length = QInputDialog::getDouble(this, "漏缆分段设计", "漏缆总长度(米):", 100, 10, 1000, 1, &ok);
    if (!ok) return;
    qreal couplingLoss = QInputDialog::getDouble(this, "漏缆分段设计", "耦合损耗(dB):", 70, 50, 100, 1, &ok);
    if (!ok) return;

    Zhifen::LeakyCableParams params;
    params.totalLength = length;
    params.couplingLoss = couplingLoss;
    params.transmissionLoss = 2.5;
    params.txPower = 43.0;
    params.targetPower = -80.0;
    params.minPower = -90.0;
    params.band = Zhifen::Band_4G;

    Zhifen::LeakyCableResult result = Zhifen::LeakyCableTool::calculate(params);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("漏缆分段设计方案");
    dlg->resize(550, 450);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    layout->addWidget(textEdit);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("漏缆分段设计: %1米, %2段").arg(length).arg(result.segmentCount));
}

void MainWindow::onBuildingToBuildingTool()
{
    bool ok;
    qreal distance = QInputDialog::getDouble(this, "楼间对打设计", "楼间距(米):", 50, 10, 500, 1, &ok);
    if (!ok) return;
    qreal txPower = QInputDialog::getDouble(this, "楼间对打设计", "发射功率(dBm):", 43, 20, 60, 1, &ok);
    if (!ok) return;

    Zhifen::BuildingToBuildingParams params;
    params.distance = distance;
    params.txPower = txPower;
    params.txHeight = 30.0;
    params.rxHeight = 15.0;
    params.txAntennaGain = 12.0;
    params.rxAntennaGain = 2.0;
    params.buildingPenetration = 15.0;
    params.targetPower = -90.0;
    params.band = Zhifen::Band_4G;
    params.antennaType = "射灯天线";

    Zhifen::BuildingToBuildingResult result = Zhifen::BuildingToBuildingTool::calculate(params);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("楼间对打设计方案");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    layout->addWidget(textEdit);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("楼间对打设计: 距离%1米, 接收%2dBm").arg(distance).arg(result.receivedPower, 0, 'f', 1));
}

void MainWindow::onAddDimension(Zhifen::DimensionType type)
{
    // 简化：在场景中心创建示例标注
    QPointF center = m_view->mapToScene(m_view->viewport()->rect().center());

    if (type == Zhifen::Dim_Linear) {
        Zhifen::LinearDimension *dim = new Zhifen::LinearDimension();
        dim->setPoints(center + QPointF(-50, 0), center + QPointF(50, 0), center + QPointF(0, 30));
        dim->setHorizontal(true);
        m_scene->addItem(dim);
        statusBar()->showMessage("已添加线性标注，可拖动调整位置", 3000);
    } else if (type == Zhifen::Dim_Aligned) {
        Zhifen::AlignedDimension *dim = new Zhifen::AlignedDimension();
        dim->setPoints(center + QPointF(-50, -20), center + QPointF(50, 20), center + QPointF(0, 40));
        m_scene->addItem(dim);
        statusBar()->showMessage("已添加对齐标注，可拖动调整位置", 3000);
    } else if (type == Zhifen::Dim_Radius) {
        Zhifen::RadiusDimension *dim = new Zhifen::RadiusDimension();
        dim->setCircle(center, 30, center + QPointF(30, 30));
        m_scene->addItem(dim);
        statusBar()->showMessage("已添加半径标注，可拖动调整位置", 3000);
    } else if (type == Zhifen::Dim_Diameter) {
        Zhifen::DiameterDimension *dim = new Zhifen::DiameterDimension();
        dim->setCircle(center, 30, center + QPointF(30, 0));
        m_scene->addItem(dim);
        statusBar()->showMessage("已添加直径标注，可拖动调整位置", 3000);
    } else if (type == Zhifen::Dim_Angular) {
        Zhifen::AngularDimension *dim = new Zhifen::AngularDimension();
        dim->setLines(center, center + QPointF(50, 0), center + QPointF(30, 40));
        m_scene->addItem(dim);
        statusBar()->showMessage("已添加角度标注，可拖动调整位置", 3000);
    }

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("添加标注: 类型%1").arg(type));
}

void MainWindow::onSheetSetManager()
{
    Zhifen::SheetSetManager &mgr = Zhifen::SheetSetManager::instance();

    // 如果图纸集为空，添加默认图纸
    if (mgr.sheetCount() == 0) {
        Zhifen::SheetInfo sheet;
        sheet.name = "一层平面布置图";
        sheet.projectName = "室分设计项目";
        sheet.designer = "设计";
        sheet.reviewer = "审核";
        sheet.date = QDate::currentDate().toString("yyyy-MM-dd");
        sheet.operatorName = "中国移动";
        sheet.scale = "1:100";
        mgr.addSheet(sheet);

        Zhifen::SheetInfo sheet2;
        sheet2.name = "系统图";
        sheet2.projectName = "室分设计项目";
        sheet2.designer = "设计";
        sheet2.reviewer = "审核";
        sheet2.date = QDate::currentDate().toString("yyyy-MM-dd");
        sheet2.operatorName = "中国移动";
        sheet2.scale = "1:50";
        mgr.addSheet(sheet2);

        // 自动编号
        Zhifen::SheetNumberConfig config;
        config.prefix = "ZF";
        config.year = "2026";
        config.startNumber = 1;
        config.digits = 3;
        mgr.autoNumber(config);
    }

    // 图纸集管理对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("图纸集管理 / 批量出图");
    dlg->resize(600, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 图纸列表
    QListWidget *listWidget = new QListWidget(dlg);
    for (int i = 0; i < mgr.sheetCount(); i++) {
        Zhifen::SheetInfo s = mgr.sheet(i);
        QListWidgetItem *item = new QListWidgetItem(QString("%1. %2 [%3]").arg(i + 1).arg(s.name).arg(s.sheetNo));
        item->setCheckState(s.selected ? Qt::Checked : Qt::Unchecked);
        listWidget->addItem(item);
    }
    layout->addWidget(listWidget);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("添加图纸", dlg);
    QPushButton *delBtn = new QPushButton("删除图纸", dlg);
    QPushButton *autoNumBtn = new QPushButton("自动编号", dlg);
    QPushButton *printBtn = new QPushButton("批量打印", dlg);
    QPushButton *pdfBtn = new QPushButton("批量导出PDF", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addWidget(autoNumBtn);
    btnLayout->addWidget(printBtn);
    btnLayout->addWidget(pdfBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    connect(addBtn, &QPushButton::clicked, this, [this, &mgr, listWidget]() {
        bool ok;
        QString name = QInputDialog::getText(this, "添加图纸", "图纸名称:", QLineEdit::Normal, "", &ok);
        if (ok && !name.isEmpty()) {
            Zhifen::SheetInfo sheet;
            sheet.name = name;
            sheet.projectName = "室分设计项目";
            sheet.designer = "设计";
            sheet.reviewer = "审核";
            sheet.date = QDate::currentDate().toString("yyyy-MM-dd");
            sheet.operatorName = "中国移动";
            sheet.scale = "1:100";
            mgr.addSheet(sheet);
            QListWidgetItem *item = new QListWidgetItem(QString("%1. %2").arg(mgr.sheetCount()).arg(name));
            item->setCheckState(Qt::Checked);
            listWidget->addItem(item);
        }
    });

    connect(delBtn, &QPushButton::clicked, this, [&mgr, listWidget]() {
        int row = listWidget->currentRow();
        if (row >= 0) {
            mgr.removeSheet(row);
            delete listWidget->takeItem(row);
        }
    });

    connect(autoNumBtn, &QPushButton::clicked, this, [&mgr, listWidget]() {
        Zhifen::SheetNumberConfig config;
        config.prefix = "ZF";
        config.year = "2026";
        config.startNumber = 1;
        config.digits = 3;
        mgr.autoNumber(config);
        listWidget->clear();
        for (int i = 0; i < mgr.sheetCount(); i++) {
            Zhifen::SheetInfo s = mgr.sheet(i);
            QListWidgetItem *item = new QListWidgetItem(QString("%1. %2 [%3]").arg(i + 1).arg(s.name).arg(s.sheetNo));
            item->setCheckState(s.selected ? Qt::Checked : Qt::Unchecked);
            listWidget->addItem(item);
        }
    });

    connect(printBtn, &QPushButton::clicked, this, [this, &mgr, listWidget]() {
        // 更新选中状态
        for (int i = 0; i < listWidget->count(); i++) {
            Zhifen::SheetInfo s = mgr.sheet(i);
            s.selected = listWidget->item(i)->checkState() == Qt::Checked;
            mgr.updateSheet(i, s);
        }
        mgr.printAll(m_scene, this);
    });

    connect(pdfBtn, &QPushButton::clicked, this, [this, &mgr, listWidget]() {
        // 更新选中状态
        for (int i = 0; i < listWidget->count(); i++) {
            Zhifen::SheetInfo s = mgr.sheet(i);
            s.selected = listWidget->item(i)->checkState() == Qt::Checked;
            mgr.updateSheet(i, s);
        }
        QString fileName = QFileDialog::getSaveFileName(this, "批量导出PDF", "图纸集.pdf", "PDF文件 (*.pdf)");
        if (!fileName.isEmpty()) {
            if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) fileName += ".pdf";
            if (mgr.exportAllToPdf(m_scene, fileName, true)) {
                QMessageBox::information(this, "导出成功", QString("图纸集已导出到: %1").arg(fileName));
            } else {
                QMessageBox::warning(this, "导出失败", "PDF导出失败");
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("图纸集管理: %1张图纸").arg(mgr.sheetCount()));
}

void MainWindow::onCreateBlock()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "创建块", "请先选择要创建为块的图元");
        return;
    }

    bool ok;
    QString name = QInputDialog::getText(this, "创建块", "块名称:", QLineEdit::Normal, "Block1", &ok);
    if (!ok || name.isEmpty()) return;

    QPointF basePoint = QPointF(0, 0);
    if (!selected.isEmpty()) {
        basePoint = selected.first()->pos();
    }

    QString blockName = Zhifen::BlockManager::instance().createBlock(name, basePoint, selected, "用户创建块");
    if (!blockName.isEmpty()) {
        // 将选中图元替换为块引用
        for (QGraphicsItem *item : selected) {
            m_scene->removeItem(item);
        }
        QGraphicsItem *blockRef = Zhifen::BlockManager::instance().createBlockReference(blockName, basePoint);
        if (blockRef) m_scene->addItem(blockRef);
        QMessageBox::information(this, "创建块成功", QString("块 '%1' 已创建，包含 %2 个图元").arg(blockName).arg(selected.size()));
        Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("创建块: %1").arg(blockName));
    } else {
        QMessageBox::warning(this, "创建块失败", "块名称已存在或创建失败");
    }
}

void MainWindow::onInsertBlock()
{
    Zhifen::BlockManager &mgr = Zhifen::BlockManager::instance();
    QList<QString> names = mgr.blockNames();
    if (names.isEmpty()) {
        QMessageBox::information(this, "插入块", "当前没有可用的块，请先创建块");
        return;
    }

    bool ok;
    QString name = QInputDialog::getItem(this, "插入块", "选择块:", names, 0, false, &ok);
    if (!ok || name.isEmpty()) return;

    QGraphicsItem *blockRef = mgr.createBlockReference(name, m_view->mapToScene(m_view->viewport()->rect().center()));
    if (blockRef) {
        m_scene->addItem(blockRef);
        Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("插入块: %1").arg(name));
    }
}

void MainWindow::onBlockManager()
{
    Zhifen::BlockManager &mgr = Zhifen::BlockManager::instance();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("块管理器");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QListWidget *listWidget = new QListWidget(dlg);
    for (const QString &name : mgr.blockNames()) {
        Zhifen::BlockDefinition *block = mgr.block(name);
        QString info = QString("%1 (%2个图元, %3个属性)")
            .arg(name).arg(block ? block->entities.size() : 0)
            .arg(block ? block->attributes.size() : 0);
        listWidget->addItem(info);
    }
    layout->addWidget(listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *insertBtn = new QPushButton("插入", dlg);
    QPushButton *renameBtn = new QPushButton("重命名", dlg);
    QPushButton *deleteBtn = new QPushButton("删除", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(insertBtn);
    btnLayout->addWidget(renameBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(insertBtn, &QPushButton::clicked, this, [this, &mgr, listWidget, dlg]() {
        int row = listWidget->currentRow();
        if (row >= 0) {
            QString name = mgr.blockNames()[row];
            QGraphicsItem *blockRef = mgr.createBlockReference(name, QPointF(0, 0));
            if (blockRef) m_scene->addItem(blockRef);
            dlg->accept();
        }
    });
    connect(deleteBtn, &QPushButton::clicked, this, [&mgr, listWidget]() {
        int row = listWidget->currentRow();
        if (row >= 0) {
            QString name = mgr.blockNames()[row];
            mgr.removeBlock(name);
            delete listWidget->takeItem(row);
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onModelSpace()
{
    Zhifen::LayoutManager::instance().enterModelSpace();
    m_view->setScene(m_scene);
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    statusBar()->showMessage("当前: 模型空间", 3000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "切换到模型空间");
}

void MainWindow::onLayoutSpace()
{
    Zhifen::LayoutManager &lm = Zhifen::LayoutManager::instance();
    if (lm.layoutCount() == 0) {
        lm.addLayout("布局1", "A3");
    }
    QString layoutName = lm.layoutNames().first();
    lm.enterLayout(layoutName);
    Zhifen::Layout *layout = lm.layout(layoutName);
    if (layout && layout->scene) {
        m_view->setScene(layout->scene);
        m_view->fitInView(layout->scene->sceneRect(), Qt::KeepAspectRatio);
    }
    statusBar()->showMessage(QString("当前: 布局空间 [%1]").arg(layoutName), 3000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("切换到布局空间: %1").arg(layoutName));
}

void MainWindow::onVersionManager()
{
    Zhifen::VersionManager &vm = Zhifen::VersionManager::instance();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("版本管理");
    dlg->resize(600, 450);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QListWidget *listWidget = new QListWidget(dlg);
    for (Zhifen::VersionSnapshot *v : vm.allVersions()) {
        QString info = QString("V%1: %2 [%3] 图元:%4 器件:%5 %6")
            .arg(v->versionNo).arg(v->name)
            .arg(v->created.toString("MM-dd hh:mm"))
            .arg(v->entityCount).arg(v->deviceCount)
            .arg(v->isCurrent ? "(当前)" : "");
        listWidget->addItem(info);
    }
    layout->addWidget(listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("保存版本", dlg);
    QPushButton *rollbackBtn = new QPushButton("回滚版本", dlg);
    QPushButton *compareBtn = new QPushButton("版本对比", dlg);
    QPushButton *deleteBtn = new QPushButton("删除版本", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(rollbackBtn);
    btnLayout->addWidget(compareBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    connect(saveBtn, &QPushButton::clicked, this, [this, &vm, listWidget, dlg]() {
        bool ok;
        QString name = QInputDialog::getText(this, "保存版本", "版本名称:", QLineEdit::Normal,
            QString("V%1").arg(vm.versionCount() + 1), &ok);
        if (!ok || name.isEmpty()) return;
        QString remark = QInputDialog::getText(this, "保存版本", "版本备注:", QLineEdit::Normal, "", &ok);
        int vno = vm.saveVersion(m_scene, name, remark);
        if (vno > 0) {
            QMessageBox::information(this, "保存成功", QString("版本 V%1 已保存").arg(vno));
            dlg->accept();
        }
    });

    connect(rollbackBtn, &QPushButton::clicked, this, [this, &vm, listWidget, dlg]() {
        int row = listWidget->currentRow();
        if (row < 0) { QMessageBox::warning(this, "回滚", "请选择要回滚的版本"); return; }
        int vno = vm.allVersions()[row]->versionNo;
        if (QMessageBox::question(this, "确认回滚", QString("确定回滚到 V%1？当前版本将丢失未保存内容").arg(vno))
            != QMessageBox::Yes) return;
        if (vm.rollbackToVersion(vno, m_scene)) {
            QMessageBox::information(this, "回滚成功", QString("已回滚到 V%1").arg(vno));
            dlg->accept();
        }
    });

    connect(compareBtn, &QPushButton::clicked, this, [&vm, listWidget]() {
        if (vm.versionCount() < 2) { QMessageBox::information(nullptr, "对比", "至少需要2个版本"); return; }
        int row = listWidget->currentRow();
        if (row < 0) row = 0;
        int v1 = vm.allVersions().first()->versionNo;
        int v2 = vm.allVersions()[row]->versionNo;
        QString result = vm.compareVersions(v1, v2);
        QMessageBox::information(nullptr, "版本对比", result);
    });

    connect(deleteBtn, &QPushButton::clicked, this, [&vm, listWidget]() {
        int row = listWidget->currentRow();
        if (row < 0) return;
        int vno = vm.allVersions()[row]->versionNo;
        if (vm.deleteVersion(vno)) {
            delete listWidget->takeItem(row);
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onChangeLog()
{
    Zhifen::ChangeReviewManager &crm = Zhifen::ChangeReviewManager::instance();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("变更记录");
    dlg->resize(700, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(crm.changeReport());
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *refreshBtn = new QPushButton("刷新", dlg);
    QPushButton *exportBtn = new QPushButton("导出报告", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(refreshBtn, &QPushButton::clicked, this, [&crm, textEdit]() {
        textEdit->setPlainText(crm.changeReport());
    });
    connect(exportBtn, &QPushButton::clicked, this, [this, &crm]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出变更报告", "变更报告.txt", "文本文件 (*.txt)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << crm.changeReport();
                file.close();
                QMessageBox::information(this, "导出成功", "变更报告已导出");
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onDesignReview()
{
    Zhifen::ChangeReviewManager &crm = Zhifen::ChangeReviewManager::instance();
    if (crm.allReviews().isEmpty()) {
        crm.initReview();
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("设计校审");
    dlg->resize(600, 450);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 三级校审状态
    QStringList levels = {"设计", "校对", "审核"};
    for (int i = 0; i < 3; i++) {
        Zhifen::ReviewRecord rec = crm.reviewRecord(static_cast<Zhifen::ReviewLevel>(i));
        QString statusStr;
        switch (rec.status) {
            case Zhifen::ReviewStatus_Pending: statusStr = "待校审"; break;
            case Zhifen::ReviewStatus_Passed: statusStr = "通过"; break;
            case Zhifen::ReviewStatus_Rejected: statusStr = "驳回"; break;
        }
        QGroupBox *group = new QGroupBox(QString("%1级 - %2 [%3]").arg(i + 1).arg(levels[i]).arg(statusStr), dlg);
        QVBoxLayout *gLayout = new QVBoxLayout(group);
        QLabel *infoLabel = new QLabel(QString("校审人: %1 | 时间: %2")
            .arg(rec.reviewer.isEmpty() ? "未填写" : rec.reviewer)
            .arg(rec.time.isNull() ? "未校审" : rec.time.toString("yyyy-MM-dd hh:mm")), group);
        QLabel *opinionLabel = new QLabel(QString("意见: %1").arg(rec.opinion.isEmpty() ? "无" : rec.opinion), group);
        opinionLabel->setWordWrap(true);
        gLayout->addWidget(infoLabel);
        gLayout->addWidget(opinionLabel);
        layout->addWidget(group);
    }

    // 提交校审
    QGroupBox *submitGroup = new QGroupBox("提交校审", dlg);
    QFormLayout *fLayout = new QFormLayout(submitGroup);
    QComboBox *levelCombo = new QComboBox(submitGroup);
    levelCombo->addItems(levels);
    QLineEdit *reviewerEdit = new QLineEdit(submitGroup);
    QTextEdit *opinionEdit = new QTextEdit(submitGroup);
    opinionEdit->setMaximumHeight(60);
    QComboBox *statusCombo = new QComboBox(submitGroup);
    statusCombo->addItems({"通过", "驳回"});
    fLayout->addRow("级别:", levelCombo);
    fLayout->addRow("校审人:", reviewerEdit);
    fLayout->addRow("意见:", opinionEdit);
    fLayout->addRow("结果:", statusCombo);
    layout->addWidget(submitGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *submitBtn = new QPushButton("提交校审", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(submitBtn, &QPushButton::clicked, this, [&crm, levelCombo, reviewerEdit, opinionEdit, statusCombo, dlg]() {
        Zhifen::ReviewLevel level = static_cast<Zhifen::ReviewLevel>(levelCombo->currentIndex());
        Zhifen::ReviewStatus status = statusCombo->currentIndex() == 0 ? Zhifen::ReviewStatus_Passed : Zhifen::ReviewStatus_Rejected;
        crm.submitReview(level, reviewerEdit->text(), opinionEdit->toPlainText(), status);
        QMessageBox::information(nullptr, "提交成功", "校审意见已提交");
        dlg->accept();
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onInterferenceAnalysis()
{
    Zhifen::NetworkPlanningTools &tools = Zhifen::NetworkPlanningTools::instance();
    QList<Zhifen::NetworkFrequencyBand> bands = tools.defaultBands();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("干扰分析");
    dlg->resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));

    // 生成干扰矩阵和详情
    QString report = tools.interferenceMatrix(bands);
    report += "\n=== 干扰详情（非无干扰项）===\n";
    QList<Zhifen::InterferenceResult> results = tools.analyzeInterference(bands);
    for (const Zhifen::InterferenceResult &r : results) {
        if (r.level != Zhifen::Interf_None) {
            report += "\n" + r.description;
            report += "\n  所需隔离度: " + QString::number(r.isolationRequired, 'f', 1) + " dB";
            report += "\n  建议: " + r.recommendation;
        }
    }
    textEdit->setPlainText(report);
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出报告", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(exportBtn, &QPushButton::clicked, this, [this, textEdit]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出干扰分析报告", "干扰分析报告.txt", "文本文件 (*.txt)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << textEdit->toPlainText();
                file.close();
                QMessageBox::information(this, "导出成功", "干扰分析报告已导出");
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "干扰分析");
}

void MainWindow::onCapacityPlanning()
{
    Zhifen::NetworkPlanningTools &tools = Zhifen::NetworkPlanningTools::instance();

    bool ok;
    qreal area = QInputDialog::getDouble(this, "容量规划", "覆盖面积(m²):", 10000, 100, 10000000, 0, &ok);
    if (!ok) return;
    qreal density = QInputDialog::getDouble(this, "容量规划", "用户密度(人/km²):", 10000, 100, 100000, 0, &ok);
    if (!ok) return;
    int cellCount = QInputDialog::getInt(this, "容量规划", "小区数量:", 4, 1, 100, 1, &ok);
    if (!ok) return;

    Zhifen::CapacityParams params;
    params.area = area;
    params.userDensity = density;
    params.voiceTrafficPerUser = 0.02;
    params.dataTrafficPerUser = 0.05;
    params.penetrationRate = 0.85;
    params.cellCount = cellCount;

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("容量规划");
    dlg->resize(700, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));

    QString report = "=== 容量规划结果 ===\n\n";
    report += QString("覆盖面积: %1 m²\n").arg(area);
    report += QString("用户密度: %1 人/km²\n").arg(density);
    report += QString("小区数量: %1\n\n").arg(cellCount);

    QList<Zhifen::NetworkStandard> standards = {Zhifen::Net_2G_GSM, Zhifen::Net_4G_LTE, Zhifen::Net_5G_NR};
    QStringList stdNames = {"2G GSM", "4G LTE", "5G NR"};
    QList<Zhifen::CapacityResult> results = tools.multiStandardCapacity(standards, params);

    for (int i = 0; i < results.size(); i++) {
        const Zhifen::CapacityResult &c = results[i];
        report += QString("[%1]\n").arg(stdNames[i]);
        report += QString("  总用户: %1  每小区: %2\n").arg(c.totalUsers).arg(c.usersPerCell);
        report += QString("  语音: %1 Erl (容量%2, 利用率%3%)\n")
            .arg(c.voiceTraffic, 0, 'f', 2).arg(c.voiceCapacity, 0, 'f', 2).arg(c.voiceUtilization, 0, 'f', 1);
        report += QString("  数据: %1 Mbps (容量%2, 利用率%3%)\n")
            .arg(c.dataTraffic, 0, 'f', 2).arg(c.dataCapacity, 0, 'f', 2).arg(c.dataUtilization, 0, 'f', 1);
        report += QString("  结论: %1\n\n").arg(c.recommendation);
    }

    textEdit->setPlainText(report);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "容量规划");
}

void MainWindow::onFrequencyPlanning()
{
    Zhifen::NetworkPlanningTools &tools = Zhifen::NetworkPlanningTools::instance();
    QList<Zhifen::NetworkFrequencyBand> bands = tools.defaultBands();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("频率规划");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));

    QString report = tools.frequencyPlanReport(bands);

    // PCI规划
    report += "\n=== 5G NR PCI规划 ===\n";
    QList<Zhifen::PCIResult> pciResults = tools.planPCI(6);
    for (const Zhifen::PCIResult &p : pciResults) {
        report += QString("  小区%1: PCI=%2 (模3=%3, 模30=%4) %5\n")
            .arg(p.cellId).arg(p.pci).arg(p.mod3).arg(p.mod30).arg(p.conflictStatus);
    }
    report += QString("  PCI冲突检查: %1\n").arg(tools.checkPCIConflict(pciResults) ? "存在冲突" : "无冲突");

    textEdit->setPlainText(report);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "频率规划");
}

void MainWindow::onPerformanceSettings()
{
    Zhifen::PerformanceManager &pm = Zhifen::PerformanceManager::instance();
    Zhifen::PerformanceSettings settings = pm.settings();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("性能设置");
    dlg->resize(450, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QGroupBox *group = new QGroupBox("渲染优化", dlg);
    QVBoxLayout *gLayout = new QVBoxLayout(group);
    QCheckBox *cullCheck = new QCheckBox("视口裁剪（只渲染可见区域）", group);
    cullCheck->setChecked(settings.viewportCulling);
    QCheckBox *lodCheck = new QCheckBox("LOD细节层次（缩小时简化绘制）", group);
    lodCheck->setChecked(settings.lodEnabled);
    QCheckBox *cacheCheck = new QCheckBox("图元缓存（复杂图元使用缓存）", group);
    cacheCheck->setChecked(settings.itemCaching);
    gLayout->addWidget(cullCheck);
    gLayout->addWidget(lodCheck);
    gLayout->addWidget(cacheCheck);
    layout->addWidget(group);

    QGroupBox *group2 = new QGroupBox("文件优化", dlg);
    QVBoxLayout *g2Layout = new QVBoxLayout(group2);
    QCheckBox *incSaveCheck = new QCheckBox("增量保存（只保存变更部分）", group2);
    incSaveCheck->setChecked(settings.incrementalSave);
    QCheckBox *bgLoadCheck = new QCheckBox("后台加载（大文件不卡UI）", group2);
    bgLoadCheck->setChecked(settings.backgroundLoading);
    QCheckBox *memCheck = new QCheckBox("内存优化（共享图元数据）", group2);
    memCheck->setChecked(settings.memoryOptimization);
    g2Layout->addWidget(incSaveCheck);
    g2Layout->addWidget(bgLoadCheck);
    g2Layout->addWidget(memCheck);
    layout->addWidget(group2);

    QGroupBox *group3 = new QGroupBox("高级设置", dlg);
    QFormLayout *fLayout = new QFormLayout(group3);
    QSpinBox *maxItemsSpin = new QSpinBox(group3);
    maxItemsSpin->setRange(1000, 50000);
    maxItemsSpin->setValue(settings.maxItemsPerFrame);
    maxItemsSpin->setSingleStep(1000);
    fLayout->addRow("每帧最大绘制:", maxItemsSpin);
    layout->addWidget(group3);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *applyBtn = new QPushButton("应用", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(applyBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(applyBtn, &QPushButton::clicked, this, [&pm, cullCheck, lodCheck, cacheCheck, incSaveCheck, bgLoadCheck, memCheck, maxItemsSpin, dlg]() {
        Zhifen::PerformanceSettings s;
        s.viewportCulling = cullCheck->isChecked();
        s.lodEnabled = lodCheck->isChecked();
        s.itemCaching = cacheCheck->isChecked();
        s.incrementalSave = incSaveCheck->isChecked();
        s.backgroundLoading = bgLoadCheck->isChecked();
        s.memoryOptimization = memCheck->isChecked();
        s.maxItemsPerFrame = maxItemsSpin->value();
        pm.setSettings(s);
        QMessageBox::information(nullptr, "应用成功", "性能设置已应用");
        dlg->accept();
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onPerformanceMonitor()
{
    Zhifen::PerformanceManager &pm = Zhifen::PerformanceManager::instance();
    QRectF viewRect = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
    pm.updateStats(m_scene, viewRect);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("性能监控");
    dlg->resize(400, 350);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(pm.statsReport());
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *refreshBtn = new QPushButton("刷新", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(refreshBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(refreshBtn, &QPushButton::clicked, this, [&pm, textEdit, this]() {
        QRectF vr = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();
        pm.updateStats(m_scene, vr);
        textEdit->setPlainText(pm.statsReport());
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onPerformanceTest()
{
    Zhifen::PerformanceManager &pm = Zhifen::PerformanceManager::instance();
    QRectF viewRect = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("性能测试");
    dlg->resize(500, 450);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(pm.runPerformanceTest(m_scene, viewRect));
    layout->addWidget(textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出报告", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(exportBtn, &QPushButton::clicked, this, [this, textEdit]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出性能测试报告", "性能测试报告.txt", "文本文件 (*.txt)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << textEdit->toPlainText();
                file.close();
                QMessageBox::information(this, "导出成功", "性能测试报告已导出");
            }
        }
    });

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "性能测试");
}

void MainWindow::onImportTianyue()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入天越格式", "", "天越文件 (*.tyd *.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.importTianYue(fileName, m_scene);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("导入结果");
    dlg->resize(500, 400);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(report.toString());
    layout->addWidget(textEdit);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入天越格式: %1").arg(fileName));
}

void MainWindow::onImportAIDP()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入AIDP格式", "", "AIDP文件 (*.aidp *.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.importAIDP(fileName, m_scene);

    QMessageBox::information(this, "导入结果", report.toString());
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入AIDP格式: %1").arg(fileName));
}

void MainWindow::onImportDifu()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入迪弗格式", "", "迪弗文件 (*.dfd *.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.importDiFu(fileName, m_scene);

    QMessageBox::information(this, "导入结果", report.toString());
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入迪弗格式: %1").arg(fileName));
}

void MainWindow::onExportTianyue()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出天越格式", "设计图纸.dxf", "天越文件 (*.tyd *.dxf)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".dxf", Qt::CaseInsensitive) && !fileName.endsWith(".tyd", Qt::CaseInsensitive)) {
        fileName += ".dxf";
    }

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.exportTianYue(m_scene, fileName);

    QMessageBox::information(this, "导出结果", report.toString());
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导出天越格式: %1").arg(fileName));
}

void MainWindow::onExportAIDP()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出AIDP格式", "设计图纸.dxf", "AIDP文件 (*.aidp *.dxf)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".dxf", Qt::CaseInsensitive) && !fileName.endsWith(".aidp", Qt::CaseInsensitive)) {
        fileName += ".dxf";
    }

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.exportAIDP(m_scene, fileName);

    QMessageBox::information(this, "导出结果", report.toString());
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导出AIDP格式: %1").arg(fileName));
}













void MainWindow::onToggleCopyMode()
{
    Zhifen::CopyModeManager &mgr = Zhifen::CopyModeManager::instance();
    if (mgr.isLightCopy()) {
        mgr.setMode(Zhifen::FULL_COPY);
        m_copyModeAct->setText("复制模式:完整");
        m_copyModeAct->setChecked(true);
    } else {
        mgr.setMode(Zhifen::LIGHT_COPY);
        m_copyModeAct->setText("复制模式:轻量");
        m_copyModeAct->setChecked(false);
    }
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_CopyModeSwitch,
        QString("切换复制模式为: %1").arg(mgr.modeName()));
    statusBar()->showMessage(QString("当前复制模式: %1 - %2").arg(mgr.modeName()).arg(mgr.modeDescription()), 5000);
}

void MainWindow::onAuditLog()
{
    QString report = Zhifen::AuditLogger::instance().toTextReport();
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("审计日志");
    dlg->resize(700, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QTextEdit *text = new QTextEdit(dlg);
    text->setReadOnly(true);
    text->setFont(QFont("Consolas", 9));
    text->setPlainText(report);
    layout->addWidget(text);
    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}
void MainWindow::onZoomIn() { m_view->zoomIn(); }
void MainWindow::onZoomOut() { m_view->zoomOut(); }

void MainWindow::onToggleGrid() { m_scene->setShowGrid(m_gridAct->isChecked()); }
void MainWindow::onToggleSnap() { if (m_view->snapManager()) m_view->snapManager()->setEnabled(m_snapAct->isChecked()); }
void MainWindow::onToggleOrtho() { m_view->setOrthoMode(m_orthoAct->isChecked()); }

void MainWindow::onCoordinateChanged(const QPointF &pos)
{
    m_coordLabel->setText(QString("X: %1  Y: %2").arg(pos.x(), 0, 'f', 2).arg(pos.y(), 0, 'f', 2));
}

void MainWindow::onCommandEntered(const QString &command)
{
    QString cmd = command.toLower().trimmed();
    if (cmd == "line" || cmd == "l") setCurrentTool("line");
    else if (cmd == "circle" || cmd == "c") setCurrentTool("circle");
    else if (cmd == "arc" || cmd == "a") setCurrentTool("arc");
    else if (cmd == "polyline" || cmd == "pl") setCurrentTool("polyline");
    else if (cmd == "rectangle" || cmd == "rec") setCurrentTool("rectangle");
    else if (cmd == "text" || cmd == "dt") setCurrentTool("text");
    else if (cmd == "dimension" || cmd == "dli" || cmd == "dim") setCurrentTool("dimension");
    else if (cmd == "move" || cmd == "m") setCurrentTool("move");
    else if (cmd == "copy" || cmd == "co") setCurrentTool("copy");
    else if (cmd == "rotate" || cmd == "ro") setCurrentTool("rotate");
    else if (cmd == "scale" || cmd == "sc") setCurrentTool("scale");
    else if (cmd == "pan" || cmd == "p") setCurrentTool("pan");
    else if (cmd == "zoom" || cmd == "z") setCurrentTool("zoom");
    else if (cmd == "select" || cmd == "v") setCurrentTool("select");
    else if (cmd == "ze" || cmd == "zoomextents") onZoomExtents();
    else if (cmd == "regen" || cmd == "re") { m_view->zoomExtents(); m_commandLine->appendMessage("已重生成", "result"); }
    else if (cmd == "clear") m_commandLine->clear();
    else if (cmd == "help") m_commandLine->appendMessage("可用命令: line, circle, pan, zoom, select, ze, regen, clear, help", "result");
    else if (cmd == "quit" || cmd == "exit") close();
    else m_commandLine->appendMessage("未知命令: " + command + "，输入 help 查看可用命令", "error");
}

void MainWindow::onToolFinished()
{
    setCurrentTool("select");
    m_selectAct->setChecked(true);
}

void MainWindow::onSelectionChanged(int count)
{
    m_selectedLabel->setText(QString("已选: %1").arg(count));
    m_entityCountLabel->setText(QString("对象: %1").arg(m_scene->items().size()));
}

void MainWindow::updateTitle()
{
    QString title = "智分Design V3.1 - " + m_document->name();
    if (m_document->isModified()) title += " *";
    setWindowTitle(title);
}
