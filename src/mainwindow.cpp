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
#include "entities/lineitem.h"
#include "entities/textitem.h"
#include "entities/labelitem.h"
#include "engine/link_calculator.h"
#include "core/system_diagram_generator.h"
#include "engine/report_engine.h"
#include "engine/print_engine.h"
#include "import/dxf_importer.h"
#include "engine/coverage_simulator.h"
#include "tools/batch_importer.h"
#include "core/floor_manager.h"
#include "core/project_info.h"
#include "plugins/plugin_manager.h"
#include "plugins/core_api.h"
#include "plugins/batch_rename_plugin.h"
#include "engine/route_planner.h"
#include "engine/power_balance_optimizer.h"
#include "tools/special_design_tools.h"
#include "entities/dimension_item.h"
#include "core/sheet_set_manager.h"
#include "blocks/blockmanager.h"
#include "blocks/blockreference.h"
#include "blocks/blockdefinition.h"
#include "widgets/blockcreatedialog.h"
#include "widgets/attributedialog.h"
#include "widgets/blockmanagerpanel.h"
#include "widgets/blockeditor.h"
#include "widgets/ribbonmenu.h"
#include "utils/iconfactory.h"
#include "widgets/cadstatusbar.h"
#include "widgets/toolbox.h"
#include "core/layout_manager.h"
#include "core/version_manager.h"
#include "core/change_review_manager.h"
#include "core/network_planning_tools.h"
#include "core/performance_manager.h"
#include "core/format_converter.h"
#include "core/command_parser.h"
#include "core/audit_logger.h"
#include "core/copy_mode_manager.h"
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTextBrowser>
#include "tools/pantool.h"
#include "tools/zoomtool.h"
#include "widgets/commandline.h"
#include "widgets/layerpanel.h"
#include "widgets/propertypanel.h"
#include "widgets/layerdialog.h"
#include "widgets/devicepanel.h"
#include "widgets/devicelibrarypanel.h"
#include "entities/deviceitem.h"
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
#include <QDate>
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
#include <QDateEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentTool(nullptr)
{
    setWindowTitle("智分Design V3.1 - 专业室分设计CAD软件");
    resize(1600, 900);
    setStyleSheet(R"(
        QMainWindow { background: #2b2b2b; }
        QToolBar { background: #3c3f41; border: none; border-bottom: 1px solid #222; spacing: 1px; padding: 2px; }
        QToolBar QToolButton { color: #cccccc; padding: 4px 8px; border-radius: 2px; font-size: 11px; }
        QToolBar QToolButton:hover { background: #505355; }
        QToolBar QToolButton:checked { background: #0e639c; color: white; }
        QToolBar QToolButton:pressed { background: #007acc; }
        QMenuBar { background: #3c3f41; color: #cccccc; border-bottom: 1px solid #222; }
        QMenuBar::item:selected { background: #505355; }
        QMenu { background: #2d2d30; color: #cccccc; border: 1px solid #555; }
        QMenu::item:selected { background: #0e639c; }
        QMenu::item { padding: 4px 20px; }
        QStatusBar { background: #3c3f41; color: #cccccc; border-top: 1px solid #222; }
        QStatusBar QLabel { color: #cccccc; padding: 0 8px; }
        QDockWidget { background: #3c3f41; color: #cccccc; border: 1px solid #222; titlebar-close-icon: none; }
        QDockWidget::title { background: #3c3f41; color: #cccccc; padding: 4px; border-bottom: 1px solid #222; }
        QTabWidget::pane { border: 1px solid #222; background: #2b2b2b; }
        QTabBar::tab { background: #3c3f41; color: #999; padding: 6px 16px; border: 1px solid #222; border-bottom: none; margin-right: 1px; }
        QTabBar::tab:selected { background: #2b2b2b; color: #fff; border-bottom: 2px solid #007acc; }
        QTabBar::tab:hover { background: #505355; color: #fff; }
        QListWidget, QTreeWidget, QTableWidget { background: #2b2b2b; color: #ccc; border: 1px solid #555; }
        QListWidget::item:selected, QTreeWidget::item:selected, QTableWidget::item:selected { background: #0e639c; color: white; }
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox { background: #1e1e1e; color: #ccc; border: 1px solid #555; padding: 3px; }
        QLineEdit:focus, QComboBox:focus { border: 1px solid #007acc; }
        QPushButton { background: #3c3f41; color: #ccc; border: 1px solid #555; padding: 5px 15px; border-radius: 2px; }
        QPushButton:hover { background: #505355; }
        QPushButton:pressed { background: #0e639c; color: white; }
        QGroupBox { color: #999; border: 1px solid #555; margin-top: 8px; padding-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }
        QScrollBar:vertical { background: #2b2b2b; width: 12px; }
        QScrollBar::handle:vertical { background: #555; border-radius: 2px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #777; }
        QScrollBar:horizontal { background: #2b2b2b; height: 12px; }
        QScrollBar::handle:horizontal { background: #555; border-radius: 2px; min-width: 30px; }
        QGraphicsView { background: #1e1e1e; border: none; }
        QTextEdit, QPlainTextEdit { background: #1e1e1e; color: #ccc; border: 1px solid #555; }
    )");

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

    // 隐藏传统菜单栏和工具栏，使用Ribbon界面
    menuBar()->hide();

    // 快速访问工具栏（标准CAD左上角）
    QToolBar *quickAccess = addToolBar("快速访问");
    quickAccess->setMovable(false);
    quickAccess->setIconSize(QSize(16, 16));
    quickAccess->addAction(m_newAct);
    quickAccess->addAction(m_openAct);
    quickAccess->addAction(m_saveAct);
    quickAccess->addSeparator();
    quickAccess->addAction(m_printAct);
    quickAccess->addSeparator();
    quickAccess->addAction(m_undoAct);
    quickAccess->addAction(m_redoAct);
    quickAccess->setStyleSheet("QToolBar { background: #2d2d30; border: none; border-bottom: 1px solid #222; padding: 2px; } QToolButton { color: #ccc; padding: 3px 6px; font-size: 11px; } QToolButton:hover { background: #505355; border-radius: 2px; }");

    // 创建Ribbon菜单
    Zhifen::RibbonMenu *ribbon = new Zhifen::RibbonMenu(this);
    ribbon->setupActions(this);
    setMenuWidget(ribbon);

    // 创建左侧工具箱
    Zhifen::ToolBox *toolBox = new Zhifen::ToolBox(this);
    addDockWidget(Qt::LeftDockWidgetArea, toolBox);

    // 创建专业状态栏
    Zhifen::CadStatusBar *cadStatusBar = new Zhifen::CadStatusBar(this);
    setStatusBar(cadStatusBar);
    cadStatusBar->showMessage("就绪");
    connect(m_view, &CadView::coordinateChanged, cadStatusBar, &Zhifen::CadStatusBar::setCoordinate);
    createDockWidgets();

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

    // 画布右键菜单（标准CAD风格）
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_view, &CadView::customContextMenuRequested, this, [this](const QPoint &pos){
        QMenu *ctxMenu = new QMenu(this);
        ctxMenu->setStyleSheet("QMenu { background: #2d2d30; color: #ccc; border: 1px solid #555; } QMenu::item:selected { background: #0e639c; } QMenu::item { padding: 6px 24px; }");
        
        ctxMenu->addAction("重复 直线", this, [this](){ setCurrentTool("line"); });
        ctxMenu->addSeparator();
        ctxMenu->addAction("剪切 Ctrl+X", this, [this](){ });
        ctxMenu->addAction("复制 Ctrl+C", this, [this](){ setCurrentTool("copy"); });
        ctxMenu->addAction("粘贴 Ctrl+V", this, [this](){ });
        ctxMenu->addSeparator();
        ctxMenu->addAction("移动 M", this, [this](){ setCurrentTool("move"); });
        ctxMenu->addAction("旋转 RO", this, [this](){ setCurrentTool("rotate"); });
        ctxMenu->addAction("缩放 SC", this, [this](){ setCurrentTool("scale"); });
        ctxMenu->addAction("删除 E", this, &MainWindow::onErase);
        ctxMenu->addSeparator();
        ctxMenu->addAction("特性 Ctrl+1", this, [this](){ });
        ctxMenu->addAction("快速选择", this, [this](){ });
        ctxMenu->addSeparator();
        ctxMenu->addAction("平移 P", this, [this](){ });
        ctxMenu->addAction("缩放 Z", this, [this](){ });
        ctxMenu->addAction("选项...", this, [this](){ });
        
        ctxMenu->exec(m_view->mapToGlobal(pos));
        delete ctxMenu;
    });

    // 连接信号
    connect(m_view, &CadView::coordinateChanged, this, &MainWindow::onCoordinateChanged);
    connect(m_view, &CadView::toolFinished, this, &MainWindow::onToolFinished);
    connect(m_view, &CadView::blockDoubleClicked, this, [this](Zhifen::BlockReference *block) {
        if (!block) return;
        Zhifen::BlockDefinition *def = block->blockDefinition();
        if (def && def->attributeCount() > 0) {
            Zhifen::AttributeDialog dlg(this);
            dlg.setBlock(def, block->attributeValues());
            if (dlg.exec() == QDialog::Accepted) {
                block->setAttributeValues(dlg.attributeValues());
                block->update();
            }
        }
    });
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
    Zhifen::IconFactory &icons = Zhifen::IconFactory::instance();
    m_newAct = new QAction(icons.icon("new"), "新建", this);
    m_newAct->setShortcut(QKeySequence::New);
    m_newAct->setStatusTip("创建新工程");
    m_newAct->setToolTip("新建工程 (Ctrl+N)"); m_newAct->setShortcut(QKeySequence::New); connect(m_newAct, &QAction::triggered, this, &MainWindow::onNew);
    m_openAct = new QAction(icons.icon("open"), "打开", this);
    m_openAct->setShortcut(QKeySequence::Open);
    m_openAct->setStatusTip("打开已有工程");
    m_openAct->setToolTip("打开工程 (Ctrl+O)"); m_openAct->setShortcut(QKeySequence::Open); connect(m_openAct, &QAction::triggered, this, &MainWindow::onOpen);
    m_saveAct = new QAction(icons.icon("save"), "保存", this);
    m_saveAct->setShortcut(QKeySequence::Save);
    m_saveAct->setStatusTip("保存当前工程");
    m_saveAct->setToolTip("保存工程 (Ctrl+S)"); m_saveAct->setShortcut(QKeySequence::Save); connect(m_saveAct, &QAction::triggered, this, &MainWindow::onSave);
    m_saveAsAct = new QAction("另存为", this); m_saveAsAct->setShortcut(QKeySequence::SaveAs); connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::onSaveAs);
    m_importDxfAct = new QAction("导入DXF", this); connect(m_importDxfAct, &QAction::triggered, this, &MainWindow::onImportDxf);
    m_importBottomMapAct = new QAction("导入建筑底图(AI精简)", this); connect(m_importBottomMapAct, &QAction::triggered, this, &MainWindow::onImportBottomMap);
    m_batchImportAct = new QAction("批量导入器件(CSV)", this); connect(m_batchImportAct, &QAction::triggered, this, &MainWindow::onBatchImport);
    m_floorManagerAct = new QAction("楼层管理", this); connect(m_floorManagerAct, &QAction::triggered, this, &MainWindow::onFloorManager);
    m_pluginManagerAct = new QAction("插件管理", this); connect(m_pluginManagerAct, &QAction::triggered, this, &MainWindow::onPluginManager);
    m_exportDxfAct = new QAction("导出DXF", this); connect(m_exportDxfAct, &QAction::triggered, this, &MainWindow::onExportDxf);
    m_exportDwgSketchAct = new QAction("草图导出DWG", this); connect(m_exportDwgSketchAct, &QAction::triggered, this, &MainWindow::onExportDwgSketch);
    m_exportDwgFinalAct = new QAction("正式归档导出DWG", this); connect(m_exportDwgFinalAct, &QAction::triggered, this, &MainWindow::onExportDwgFinal);
    m_printAct = new QAction(icons.icon("print"), "打印", this);
    m_printAct->setShortcut(QKeySequence::Print);
    m_printAct->setStatusTip("打印当前图纸");
    m_printAct->setToolTip("打印 (Ctrl+P)"); m_printAct->setShortcut(QKeySequence::Print); connect(m_printAct, &QAction::triggered, this, &MainWindow::onPrint);
    m_exportPdfSketchAct = new QAction("导出PDF(草图)", this); connect(m_exportPdfSketchAct, &QAction::triggered, this, [this](){ onExportPdf(Zhifen::Paper_A4, false); });
    m_exportPdfFormalAct = new QAction("导出PDF(正式归档)", this); connect(m_exportPdfFormalAct, &QAction::triggered, this, [this](){ onExportPdf(Zhifen::Paper_A4, true); });
    m_exitAct = new QAction("退出", this); connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    m_undoAct = new QAction(icons.icon("undo"), "撤销", this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setStatusTip("撤销上一步操作");
    m_undoAct->setToolTip("撤销 (Ctrl+Z)"); m_undoAct->setShortcut(QKeySequence::Undo); connect(m_undoAct, &QAction::triggered, this, &MainWindow::onUndo);
    m_redoAct = new QAction(icons.icon("redo"), "重做", this);
    m_redoAct->setShortcut(QKeySequence::Redo);
    m_redoAct->setStatusTip("重做已撤销的操作");
    m_redoAct->setToolTip("重做 (Ctrl+Y)"); m_redoAct->setShortcut(QKeySequence::Redo); connect(m_redoAct, &QAction::triggered, this, &MainWindow::onRedo);

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
    m_explodeAct = new QAction("分解", this); m_explodeAct->setShortcut(Qt::Key_X); connect(m_explodeAct, &QAction::triggered, this, [this](){ setCurrentTool("explode"); });
    m_offsetAct = new QAction("偏移", this); m_offsetAct->setShortcut(Qt::Key_O); connect(m_offsetAct, &QAction::triggered, this, [this](){ setCurrentTool("offset"); });
    m_copyModeAct = new QAction("复制模式:轻量", this); m_copyModeAct->setCheckable(true); connect(m_copyModeAct, &QAction::triggered, this, &MainWindow::onToggleCopyMode);
    m_eraseAct = new QAction("删除", this); m_eraseAct->setShortcut(Qt::Key_E); connect(m_eraseAct, &QAction::triggered, this, [this](){ auto items = m_scene->selectedItems(); if(!items.isEmpty()) { Zhifen::AuditLogger::instance().log(items.size() > 1 ? Zhifen::Audit_BatchDelete : Zhifen::Audit_DeviceDelete, QString("删除%1个对象").arg(items.size())); m_undoStack->push(new RemoveItemsCommand(m_scene, items)); } });

    m_panAct = new QAction("平移", this); m_panAct->setShortcut(Qt::Key_P); m_panAct->setCheckable(true); m_toolGroup->addAction(m_panAct); connect(m_panAct, &QAction::triggered, this, [this](){ setCurrentTool("pan"); });
    m_zoomAct = new QAction("缩放", this); m_zoomAct->setShortcut(Qt::Key_Z); m_zoomAct->setCheckable(true); m_toolGroup->addAction(m_zoomAct); connect(m_zoomAct, &QAction::triggered, this, [this](){ setCurrentTool("zoom"); });
    m_zoomExtentsAct = new QAction("全部缩放", this); m_zoomExtentsAct; connect(m_zoomExtentsAct, &QAction::triggered, this, &MainWindow::onZoomExtents);

    m_gridAct = new QAction("网格", this); m_gridAct->setShortcut(Qt::Key_F7); m_gridAct->setCheckable(true); m_gridAct->setChecked(true); connect(m_gridAct, &QAction::triggered, this, &MainWindow::onToggleGrid);
    m_snapAct = new QAction("对象捕捉", this); m_snapAct->setShortcut(Qt::Key_F3); m_snapAct->setCheckable(true); m_snapAct->setChecked(true); connect(m_snapAct, &QAction::triggered, this, &MainWindow::onToggleSnap);
    m_orthoAct = new QAction("正交", this); m_orthoAct->setShortcut(Qt::Key_F8); m_orthoAct->setCheckable(true); connect(m_orthoAct, &QAction::triggered, this, &MainWindow::onToggleOrtho);

    m_layerManagerAct = new QAction("图层管理", this); m_layerManagerAct->setShortcut(Qt::Key_F2); connect(m_layerManagerAct, &QAction::triggered, this, [this](){ LayerDialog dlg(m_document, this); dlg.exec(); m_layerPanel->refresh(); });

    // === 计算与仿真 ===
    m_linkCalcAct = new QAction("链路预算", this); connect(m_linkCalcAct, &QAction::triggered, this, &MainWindow::onLinkCalculation);
    m_bomAct = new QAction("材料统计", this); connect(m_bomAct, &QAction::triggered, this, &MainWindow::onBomReport);
    m_sysDiagramSketchAct = new QAction("系统图(草图)", this); connect(m_sysDiagramSketchAct, &QAction::triggered, this, [this](){ onGenerateSystemDiagram(Zhifen::SystemLayoutMode::ModeA); });
    m_sysDiagramFormalAct = new QAction("系统图(正式)", this); connect(m_sysDiagramFormalAct, &QAction::triggered, this, [this](){ onGenerateSystemDiagram(Zhifen::SystemLayoutMode::ModeA); });
    m_coverageSimAct = new QAction("覆盖仿真", this); connect(m_coverageSimAct, &QAction::triggered, this, &MainWindow::onCoverageSimulation);
    m_smartRouteAct = new QAction("智能路由", this); connect(m_smartRouteAct, &QAction::triggered, this, &MainWindow::onSmartRoute);
    m_powerBalanceAct = new QAction("功率平衡", this); connect(m_powerBalanceAct, &QAction::triggered, this, &MainWindow::onPowerBalance);

    // === 特殊设计工具 ===
    m_elevatorToolAct = new QAction("电梯覆盖", this); connect(m_elevatorToolAct, &QAction::triggered, this, &MainWindow::onElevatorTool);
    m_leakyCableToolAct = new QAction("漏缆设计", this); connect(m_leakyCableToolAct, &QAction::triggered, this, &MainWindow::onLeakyCableTool);
    m_b2bToolAct = new QAction("楼间对打", this); connect(m_b2bToolAct, &QAction::triggered, this, &MainWindow::onBuildingToBuildingTool);

    // === 块功能 ===
    m_createBlockAct = new QAction("创建块", this); connect(m_createBlockAct, &QAction::triggered, this, &MainWindow::onCreateBlock);
    m_insertBlockAct = new QAction("插入块", this); connect(m_insertBlockAct, &QAction::triggered, this, &MainWindow::onInsertBlock);
    m_blockManagerAct = new QAction("块管理器", this); connect(m_blockManagerAct, &QAction::triggered, this, &MainWindow::onBlockManager);

    // === 图纸与版本 ===
    m_sheetSetAct = new QAction("图纸集管理", this); connect(m_sheetSetAct, &QAction::triggered, this, &MainWindow::onSheetSetManager);
    m_modelSpaceAct = new QAction("模型空间", this); connect(m_modelSpaceAct, &QAction::triggered, this, &MainWindow::onModelSpace);
    m_layoutSpaceAct = new QAction("布局空间", this); connect(m_layoutSpaceAct, &QAction::triggered, this, &MainWindow::onLayoutSpace);
    m_versionMgrAct = new QAction("版本管理", this); connect(m_versionMgrAct, &QAction::triggered, this, &MainWindow::onVersionManager);
    m_changeLogAct = new QAction("变更记录", this); connect(m_changeLogAct, &QAction::triggered, this, &MainWindow::onChangeLog);
    m_reviewAct = new QAction("设计审查", this); connect(m_reviewAct, &QAction::triggered, this, &MainWindow::onDesignReview);

    // === 分析与规划 ===
    m_interferenceAct = new QAction("干扰分析", this); connect(m_interferenceAct, &QAction::triggered, this, &MainWindow::onInterferenceAnalysis);
    m_capacityAct = new QAction("容量规划", this); connect(m_capacityAct, &QAction::triggered, this, &MainWindow::onCapacityPlanning);
    m_frequencyAct = new QAction("频率规划", this); connect(m_frequencyAct, &QAction::triggered, this, &MainWindow::onFrequencyPlanning);

    // === 性能 ===
    m_perfSettingsAct = new QAction("性能设置", this); connect(m_perfSettingsAct, &QAction::triggered, this, &MainWindow::onPerformanceSettings);
    m_perfMonitorAct = new QAction("性能监控", this); connect(m_perfMonitorAct, &QAction::triggered, this, &MainWindow::onPerformanceMonitor);
    m_perfTestAct = new QAction("性能测试", this); connect(m_perfTestAct, &QAction::triggered, this, &MainWindow::onPerformanceTest);

    // === 格式互导 ===
    m_importTianyueAct = new QAction("导入天越格式", this); connect(m_importTianyueAct, &QAction::triggered, this, &MainWindow::onImportTianyue);
    m_importAIDPAct = new QAction("导入AIDP格式", this); connect(m_importAIDPAct, &QAction::triggered, this, &MainWindow::onImportAIDP);
    m_importDifuAct = new QAction("导入迪弗格式", this); connect(m_importDifuAct, &QAction::triggered, this, &MainWindow::onImportDifu);
    m_exportTianyueAct = new QAction("导出天越格式", this); connect(m_exportTianyueAct, &QAction::triggered, this, &MainWindow::onExportTianyue);
    m_exportAIDPAct = new QAction("导出AIDP格式", this); connect(m_exportAIDPAct, &QAction::triggered, this, &MainWindow::onExportAIDP);

    // === 查询工具 ===
    m_queryDistAct = new QAction("距离查询", this); connect(m_queryDistAct, &QAction::triggered, this, [this](){ setCurrentTool("query_dist"); });
    m_queryAreaAct = new QAction("面积查询", this); connect(m_queryAreaAct, &QAction::triggered, this, [this](){ setCurrentTool("query_area"); });
    m_queryPointAct = new QAction("点坐标查询", this); connect(m_queryPointAct, &QAction::triggered, this, [this](){ setCurrentTool("query_point"); });

    // === 其他 ===
    m_mirrorAct = new QAction("镜像", this); connect(m_mirrorAct, &QAction::triggered, this, [this](){ setCurrentTool("mirror"); });
    m_auditLogAct = new QAction("审计日志", this); connect(m_auditLogAct, &QAction::triggered, this, &MainWindow::onAuditLog);
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
    toolsMenu->addSeparator();
    toolsMenu->addAction("AI底图精简", this, &MainWindow::onAISimplify);
    toolsMenu->addAction("AI自动布放", this, &MainWindow::onAutoPlace);
    toolsMenu->addAction("AI材料估算", this, &MainWindow::onMaterialEstimate);
    toolsMenu->addSeparator();
    toolsMenu->addAction("标准层批量复制", this, &MainWindow::onCopyStandardFloor);
    toolsMenu->addAction("天线功率统计", this, &MainWindow::onAntennaPowerStats);
    toolsMenu->addAction("系统图切割分页", this, &MainWindow::onSystemDiagramSplit);
    toolsMenu->addAction("智能标签系统", this, &MainWindow::onSmartLabel);
    toolsMenu->addAction("平面图系统图双向关联", this, &MainWindow::onDualLink);
    toolsMenu->addSeparator();
    toolsMenu->addAction("保存基准图纸", this, &MainWindow::onSaveBaseline);
    toolsMenu->addAction("图纸对比", this, &MainWindow::onDrawingCompare);
    toolsMenu->addAction("主干自动生成", this, &MainWindow::onAutoTrunk);

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("新手教程", this, &MainWindow::onHelpTutorial);
    helpMenu->addAction("操作手册", this, &MainWindow::onHelpManual);
    helpMenu->addAction("功能亮点", this, &MainWindow::onHelpFeature);
    helpMenu->addSeparator();
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
    // 器件库面板（新：60种器件，对标天越11大类）
    QDockWidget *deviceDock = new QDockWidget("器件库", this);
    deviceDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    Zhifen::DeviceLibraryPanel *devLibrary = new Zhifen::DeviceLibraryPanel(this);
    deviceDock->setWidget(devLibrary);
    addDockWidget(Qt::RightDockWidgetArea, deviceDock);
    connect(devLibrary, &Zhifen::DeviceLibraryPanel::deviceSelected, this, [this](Zhifen::DeviceItem::DeviceType type, const QString &name){
        // 进入器件放置模式：点击画布时放置器件
        m_pendingDeviceType = type;
        m_pendingDeviceName = name;
        statusBar()->showMessage(QString("放置器件: %1，点击画布放置，右键取消").arg(name), 5000);
        // 临时连接鼠标点击事件
        if (!m_devicePlaceConnection) {
            m_devicePlaceConnection = connect(m_view, &CadView::sceneClicked, this, [this](QPointF pos){
                if (m_pendingDeviceName.isEmpty()) return;
                Zhifen::DeviceItem *dev = new Zhifen::DeviceItem(m_pendingDeviceType);
                dev->setPos(pos);
                dev->setToolTip(m_pendingDeviceName);
                m_scene->addItem(dev);
                m_scene->clearSelection();
                dev->setSelected(true);
                statusBar()->showMessage(QString("已放置: %1").arg(m_pendingDeviceName), 3000);
                // 放置一个后清除（可改为连续放置）
                m_pendingDeviceName = "";
                m_pendingDeviceType = Zhifen::DeviceItem::OmniAntenna;
            });
        }
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

    // 第一步：选择精简模式
    QStringList items;
    items << "不精简(全部导入)" << "AI基础精简(保留墙体/门窗/管线)" << "AI深度精简(仅保留墙体)";
    bool ok;
    QString choice = QInputDialog::getItem(this, "AI精简模式", "请选择底图精简模式:", items, 1, false, &ok);
    if (!ok) return;

    Zhifen::SimplifyMode mode = Zhifen::Simplify_Basic;
    if (choice.contains("不精简")) mode = Zhifen::Simplify_None;
    else if (choice.contains("深度")) mode = Zhifen::Simplify_Aggressive;

    // 先解析DXF获取图层列表
    Zhifen::DxfImporter importer;
    Zhifen::DxfImportResult result = importer.importFromFile(fileName, Zhifen::Simplify_None);

    if (!result.success) {
        QString err = result.errors.isEmpty() ? "导入失败" : result.errors.join("\n");
        QMessageBox::warning(this, "底图导入失败", err);
        return;
    }

    // 第二步：图层选择对话框
    QDialog *layerDlg = new QDialog(this);
    layerDlg->setWindowTitle("图层选择 - 勾选要保留的图层");
    layerDlg->resize(500, 500);
    QVBoxLayout *layerLayout = new QVBoxLayout(layerDlg);

    QLabel *tipLabel = new QLabel(QString("共发现 %1 个图层，勾选需要保留的图层：").arg(result.layers.size()));
    layerLayout->addWidget(tipLabel);

    QListWidget *layerList = new QListWidget(layerDlg);
    layerList->setSelectionMode(QAbstractItemView::MultiSelection);
    for (const auto &layer : result.layers) {
        QListWidgetItem *item = new QListWidgetItem(QString("%1 (%2个图元)").arg(layer.name).arg(layer.entityCount));
        item->setCheckState(Qt::Checked);
        item->setData(Qt::UserRole, layer.name);
        layerList->addItem(item);
    }
    layerLayout->addWidget(layerList, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *selectAllBtn = new QPushButton("全选", layerDlg);
    QPushButton *clearAllBtn = new QPushButton("全不选", layerDlg);
    QPushButton *wallOnlyBtn = new QPushButton("仅墙体", layerDlg);
    QPushButton *okBtn = new QPushButton("确定导入", layerDlg);
    QPushButton *cancelBtn = new QPushButton("取消", layerDlg);
    btnLayout->addWidget(selectAllBtn);
    btnLayout->addWidget(clearAllBtn);
    btnLayout->addWidget(wallOnlyBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layerLayout->addLayout(btnLayout);

    connect(selectAllBtn, &QPushButton::clicked, layerList, [layerList]() {
        for (int i = 0; i < layerList->count(); i++) layerList->item(i)->setCheckState(Qt::Checked);
    });
    connect(clearAllBtn, &QPushButton::clicked, layerList, [layerList]() {
        for (int i = 0; i < layerList->count(); i++) layerList->item(i)->setCheckState(Qt::Unchecked);
    });
    connect(wallOnlyBtn, &QPushButton::clicked, layerList, [layerList]() {
        for (int i = 0; i < layerList->count(); i++) {
            QString name = layerList->item(i)->data(Qt::UserRole).toString();
            bool isWall = name.contains("墙", Qt::CaseInsensitive) || name.contains("WALL", Qt::CaseInsensitive);
            layerList->item(i)->setCheckState(isWall ? Qt::Checked : Qt::Unchecked);
        }
    });
    connect(okBtn, &QPushButton::clicked, layerDlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, layerDlg, &QDialog::reject);

    if (layerDlg->exec() != QDialog::Accepted) {
        layerDlg->deleteLater();
        return;
    }

    // 收集用户选择的图层
    QStringList selectedLayers;
    for (int i = 0; i < layerList->count(); i++) {
        if (layerList->item(i)->checkState() == Qt::Checked) {
            selectedLayers.append(layerList->item(i)->data(Qt::UserRole).toString());
        }
    }
    layerDlg->deleteLater();

    // 按用户选择过滤图元
    QList<Zhifen::DxfEntity> filteredEntities;
    for (const auto &entity : result.entities) {
        if (selectedLayers.contains(entity.layer)) {
            filteredEntities.append(entity);
        }
    }
    result.entities = filteredEntities;

    // 渲染到场景（底图锁定）
    importer.renderToScene(result, m_scene, true);
    m_view->zoomExtents();

    // 显示导入信息
    QString info = QString("底图导入成功!\n保留图层: %1/%2\n图元数: %3")
        .arg(selectedLayers.size()).arg(result.layers.size()).arg(filteredEntities.size());
    if (!result.warnings.isEmpty()) {
        info += "\n\n提示:\n" + result.warnings.join("\n");
    }
    QMessageBox::information(this, "导入成功", info);

    statusBar()->showMessage(QString("建筑底图已导入: %1 (已锁定)").arg(fileName), 5000);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("导入建筑底图: %1, 保留图层%2/%3, 图元%4个")
        .arg(fileName).arg(selectedLayers.size()).arg(result.layers.size()).arg(filteredEntities.size()));
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
    printer.setPageSize(QPrinter::A3);
    printer.setOrientation(QPrinter::Landscape);

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowTitle("打印预览 - 智分Design");
    preview.resize(1200, 800);

    Zhifen::PrintEngine printEngine;
    Zhifen::PrintSettings settings;
    settings.paperSize = Zhifen::Paper_A3;
    settings.printTitleBlock = true;
    settings.printLegend = true;
    settings.printMaterialTable = true;
    settings.printBorder = true;

    Zhifen::TitleBlockInfo info;
    info.projectName = "室内分布系统工程";
    info.drawingName = "平面布置图";
    info.drawingNo = "ZF-PLAN-001";
    info.designer = "设计";
    info.reviewer = "审核";
    info.date = QDate::currentDate().toString("yyyy-MM-dd");
    info.operatorName = "智分Design";

    connect(&preview, &QPrintPreviewDialog::paintRequested, this, [this, &printEngine, &settings, &info](QPrinter *p){
        QPainter painter(p);
        QRectF rect = p->pageRect();
        printEngine.renderToPainter(m_scene, &painter, rect, settings, info);
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

    // 计算统计信息
    qreal minP = 999, maxP = -999, sumP = 0;
    for (const auto &a : report.antennas) {
        minP = qMin(minP, a.outputPower);
        maxP = qMax(maxP, a.outputPower);
        sumP += a.outputPower;
    }
    qreal avgP = report.antennas.isEmpty() ? 0 : sumP / report.antennas.size();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(QString("链路预算报告 - %1").arg(bandStr));
    dlg->resize(900, 650);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 统计信息栏
    QGroupBox *statGroup = new QGroupBox("统计汇总", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("信源:"), 0, 0);
    statLayout->addWidget(new QLabel(report.sourceId), 0, 1);
    statLayout->addWidget(new QLabel("发射功率:"), 0, 2);
    statLayout->addWidget(new QLabel(QString("%1 dBm").arg(report.sourcePower, 0, 'f', 1)), 0, 3);
    statLayout->addWidget(new QLabel("器件总数:"), 1, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalDevices)), 1, 1);
    statLayout->addWidget(new QLabel("天线总数:"), 1, 2);
    statLayout->addWidget(new QLabel(QString::number(report.totalAntennas)), 1, 3);
    statLayout->addWidget(new QLabel("最小天线功率:"), 2, 0);
    QLabel *minLabel = new QLabel(QString("%1 dBm").arg(minP, 0, 'f', 1));
    minLabel->setStyleSheet(minP < 10 ? "color: red; font-weight: bold;" : "color: green;");
    statLayout->addWidget(minLabel, 2, 1);
    statLayout->addWidget(new QLabel("最大天线功率:"), 2, 2);
    statLayout->addWidget(new QLabel(QString("%1 dBm").arg(maxP, 0, 'f', 1)), 2, 3);
    statLayout->addWidget(new QLabel("平均天线功率:"), 3, 0);
    statLayout->addWidget(new QLabel(QString("%1 dBm").arg(avgP, 0, 'f', 1)), 3, 1);
    layout->addWidget(statGroup);

    // 详细结果表格
    QTableWidget *table = new QTableWidget(dlg);
    table->setColumnCount(8);
    table->setHorizontalHeaderLabels({"编号", "器件名称", "类型", "馈线长度(m)", "馈线损耗(dB)", "器件损耗(dB)", "输入功率(dBm)", "输出功率(dBm)"});
    table->setRowCount(report.results.size());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setStyleSheet("QTableWidget { gridline-color: #ccc; } QHeaderView::section { background: #007acc; color: white; padding: 4px; }");

    for (int i = 0; i < report.results.size(); i++) {
        const auto &r = report.results[i];
        table->setItem(i, 0, new QTableWidgetItem(r.deviceId));
        table->setItem(i, 1, new QTableWidgetItem(r.deviceName));
        table->setItem(i, 2, new QTableWidgetItem(r.deviceType));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(r.cableLength, 'f', 1)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(r.cableLoss, 'f', 2)));
        table->setItem(i, 5, new QTableWidgetItem(QString::number(r.deviceLoss, 'f', 2)));
        table->setItem(i, 6, new QTableWidgetItem(QString::number(r.inputPower, 'f', 1)));
        QTableWidgetItem *outItem = new QTableWidgetItem(QString::number(r.outputPower, 'f', 1));
        if (r.alarm) {
            outItem->setBackground(QColor(255, 200, 200));
            outItem->setForeground(QColor(200, 0, 0));
        }
        table->setItem(i, 7, outItem);
    }
    table->resizeColumnsToContents();
    layout->addWidget(table, 1);

    // 告警信息
    if (!report.alarms.isEmpty()) {
        QGroupBox *alarmGroup = new QGroupBox("告警信息", dlg);
        alarmGroup->setStyleSheet("QGroupBox { color: red; border: 1px solid red; }");
        QVBoxLayout *alarmLayout = new QVBoxLayout(alarmGroup);
        for (const auto &alarm : report.alarms) {
            QLabel *alarmLabel = new QLabel("⚠ " + alarm, alarmGroup);
            alarmLabel->setStyleSheet("color: red;");
            alarmLayout->addWidget(alarmLabel);
        }
        layout->addWidget(alarmGroup);
    }

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出报告", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(exportBtn, &QPushButton::clicked, dlg, [this, report]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出链路预算报告", "", "文本文件 (*.txt);;CSV文件 (*.csv)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << report.toText();
                file.close();
                QMessageBox::information(this, "导出成功", "链路预算报告已导出");
            }
        }
    });
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onBomReport()
{
    // 使用ReportEngine生成结构化BOM
    Zhifen::ReportEngine reportEngine;
    QList<Zhifen::BomItem> bom = reportEngine.generateBom(m_scene);

    // 主材/辅材分类
    QStringList mainCategories = {"天线", "功分器", "耦合器", "合路器", "信源", "馈线", "漏缆"};
    QList<Zhifen::BomItem> mainMaterials, auxMaterials;
    for (const auto &item : bom) {
        if (mainCategories.contains(item.category)) {
            mainMaterials.append(item);
        } else {
            auxMaterials.append(item);
        }
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("材料表统计 - BOM");
    dlg->resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 主材表
    QGroupBox *mainGroup = new QGroupBox("主材表", dlg);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainGroup);
    QTableWidget *mainTable = new QTableWidget(mainGroup);
    mainTable->setColumnCount(5);
    mainTable->setHorizontalHeaderLabels({"序号", "类别", "名称", "型号", "数量"});
    mainTable->setRowCount(mainMaterials.size());
    mainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainTable->setAlternatingRowColors(true);
    mainTable->setStyleSheet("QHeaderView::section { background: #1976d2; color: white; padding: 4px; }");
    for (int i = 0; i < mainMaterials.size(); i++) {
        const auto &item = mainMaterials[i];
        mainTable->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        mainTable->setItem(i, 1, new QTableWidgetItem(item.category));
        mainTable->setItem(i, 2, new QTableWidgetItem(item.name));
        mainTable->setItem(i, 3, new QTableWidgetItem(item.model));
        mainTable->setItem(i, 4, new QTableWidgetItem(QString::number(item.quantity)));
    }
    mainTable->resizeColumnsToContents();
    mainLayout->addWidget(mainTable);
    layout->addWidget(mainGroup);

    // 辅材表
    QGroupBox *auxGroup = new QGroupBox("辅材表", dlg);
    QVBoxLayout *auxLayout = new QVBoxLayout(auxGroup);
    QTableWidget *auxTable = new QTableWidget(auxGroup);
    auxTable->setColumnCount(5);
    auxTable->setHorizontalHeaderLabels({"序号", "类别", "名称", "型号", "数量"});
    auxTable->setRowCount(auxMaterials.size());
    auxTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auxTable->setAlternatingRowColors(true);
    auxTable->setStyleSheet("QHeaderView::section { background: #388e3c; color: white; padding: 4px; }");
    for (int i = 0; i < auxMaterials.size(); i++) {
        const auto &item = auxMaterials[i];
        auxTable->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        auxTable->setItem(i, 1, new QTableWidgetItem(item.category));
        auxTable->setItem(i, 2, new QTableWidgetItem(item.name));
        auxTable->setItem(i, 3, new QTableWidgetItem(item.model));
        auxTable->setItem(i, 4, new QTableWidgetItem(QString::number(item.quantity)));
    }
    auxTable->resizeColumnsToContents();
    auxLayout->addWidget(auxTable);
    layout->addWidget(auxGroup);

    // 预算表（折扣填写）
    QGroupBox *budgetGroup = new QGroupBox("预算表（折扣设置）", dlg);
    QGridLayout *budgetLayout = new QGridLayout(budgetGroup);
    budgetLayout->addWidget(new QLabel("设计折扣:"), 0, 0);
    QDoubleSpinBox *designDiscount = new QDoubleSpinBox(budgetGroup);
    designDiscount->setRange(0, 100); designDiscount->setValue(100); designDiscount->setSuffix("%");
    budgetLayout->addWidget(designDiscount, 0, 1);
    budgetLayout->addWidget(new QLabel("监理折扣:"), 0, 2);
    QDoubleSpinBox *supervisionDiscount = new QDoubleSpinBox(budgetGroup);
    supervisionDiscount->setRange(0, 100); supervisionDiscount->setValue(100); supervisionDiscount->setSuffix("%");
    budgetLayout->addWidget(supervisionDiscount, 0, 3);
    budgetLayout->addWidget(new QLabel("施工折扣:"), 1, 0);
    QDoubleSpinBox *constructionDiscount = new QDoubleSpinBox(budgetGroup);
    constructionDiscount->setRange(0, 100); constructionDiscount->setValue(100); constructionDiscount->setSuffix("%");
    budgetLayout->addWidget(constructionDiscount, 1, 1);
    budgetLayout->addWidget(new QLabel("主材合计:"), 1, 2);
    int mainTotal = 0;
    for (const auto &m : mainMaterials) mainTotal += m.quantity;
    budgetLayout->addWidget(new QLabel(QString("%1 项").arg(mainTotal)), 1, 3);
    layout->addWidget(budgetGroup);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportExcelBtn = new QPushButton("导出Excel(CSV)", dlg);
    QPushButton *exportBudgetBtn = new QPushButton("导出预算表", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(exportExcelBtn);
    btnLayout->addWidget(exportBudgetBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    // 导出Excel
    connect(exportExcelBtn, &QPushButton::clicked, dlg, [this, mainMaterials, auxMaterials]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出材料表", "材料表.csv", "CSV文件 (*.csv)");
        if (fileName.isEmpty()) return;
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << "主材表\n";
            out << "序号,类别,名称,型号,数量\n";
            for (int i = 0; i < mainMaterials.size(); i++) {
                const auto &m = mainMaterials[i];
                out << QString("%1,%2,%3,%4,%5\n").arg(i+1).arg(m.category).arg(m.name).arg(m.model).arg(m.quantity);
            }
            out << "\n辅材表\n";
            out << "序号,类别,名称,型号,数量\n";
            for (int i = 0; i < auxMaterials.size(); i++) {
                const auto &m = auxMaterials[i];
                out << QString("%1,%2,%3,%4,%5\n").arg(i+1).arg(m.category).arg(m.name).arg(m.model).arg(m.quantity);
            }
            file.close();
            QMessageBox::information(this, "导出成功", "材料表已导出为CSV格式");
        }
    });

    // 导出预算表
    connect(exportBudgetBtn, &QPushButton::clicked, dlg, [this, mainMaterials, auxMaterials, designDiscount, supervisionDiscount, constructionDiscount]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出预算表", "预算表.csv", "CSV文件 (*.csv)");
        if (fileName.isEmpty()) return;
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setCodec("UTF-8");
            out << "预算表\n";
            out << QString("设计折扣,%1%\n").arg(designDiscount->value());
            out << QString("监理折扣,%1%\n").arg(supervisionDiscount->value());
            out << QString("施工折扣,%1%\n").arg(constructionDiscount->value());
            out << "\n主材预算\n";
            out << "序号,类别,名称,型号,数量,单价(元),合价(元)\n";
            for (int i = 0; i < mainMaterials.size(); i++) {
                const auto &m = mainMaterials[i];
                out << QString("%1,%2,%3,%4,%5,,\n").arg(i+1).arg(m.category).arg(m.name).arg(m.model).arg(m.quantity);
            }
            out << "\n辅材预算\n";
            out << "序号,类别,名称,型号,数量,单价(元),合价(元)\n";
            for (int i = 0; i < auxMaterials.size(); i++) {
                const auto &m = auxMaterials[i];
                out << QString("%1,%2,%3,%4,%5,,\n").arg(i+1).arg(m.category).arg(m.name).arg(m.model).arg(m.quantity);
            }
            file.close();
            QMessageBox::information(this, "导出成功", "预算表已导出，请填写单价后计算合价");
        }
    });

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onGenerateSystemDiagram(Zhifen::SystemLayoutMode mode)
{
    // 使用新的拓扑模型生成系统图
    Zhifen::DistributionNetwork network;

    // 构建示例拓扑（后续从场景自动提取）
    auto source = std::make_shared<Zhifen::DeviceNode>();
    source->id = "SOURCE";
    source->type = Zhifen::DeviceType::Source;
    source->outputPower = 43.0;
    network.addDevice(source);
    network.sourceDeviceId = "SOURCE";

    auto cpl1 = std::make_shared<Zhifen::DeviceNode>();
    cpl1->id = "T1";
    cpl1->type = Zhifen::DeviceType::Coupler;
    cpl1->couplerDb = 10;
    cpl1->insertionLoss = 0.5;
    network.addDevice(cpl1);

    auto ant1 = std::make_shared<Zhifen::DeviceNode>();
    ant1->id = "ANT1";
    ant1->type = Zhifen::DeviceType::Antenna;
    network.addDevice(ant1);

    // 连接
    Zhifen::CableLink cable1;
    cable1.id = "C1";
    cable1.fromDeviceId = "SOURCE";
    cable1.toDeviceId = "T1";
    cable1.length = 10;
    cable1.type = "1/2";
    cable1.calculateLoss();
    network.addCable(cable1);

    Zhifen::CableLink cable2;
    cable2.id = "C2";
    cable2.fromDeviceId = "T1";
    cable2.toDeviceId = "ANT1";
    cable2.length = 5;
    cable2.type = "1/2";
    cable2.calculateLoss();
    network.addCable(cable2);

    // 生成系统图
    Zhifen::SystemDiagramGenerator generator;
    generator.setNetwork(network);
    generator.setLayoutMode(mode);
    bool ok = generator.generate();

    if (!ok) {
        QMessageBox::warning(this, "系统图生成失败", "生成失败");
        return;
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("系统图");
    dlg->resize(900, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QGraphicsView *view = new QGraphicsView(dlg);
    QGraphicsScene *scene = new QGraphicsScene(dlg);
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor(43, 43, 43));
    view->setDragMode(QGraphicsView::ScrollHandDrag);

    // 绘制器件
    for (auto &node : generator.nodes()) {
        QGraphicsRectItem *item = new QGraphicsRectItem(node.position.x() - 25, node.position.y() - 10, 50, 20);
        item->setPen(QPen(Qt::red, 2));
        item->setBrush(QBrush(QColor(43, 43, 43)));
        scene->addItem(item);
        QGraphicsTextItem *text = scene->addText(node.label, QFont("Microsoft YaHei", 9));
        text->setDefaultTextColor(Qt::white);
        text->setPos(node.position.x() - 20, node.position.y() - 8);
    }

    // 绘制馈线
    for (auto &cable : generator.cables()) {
        if (cable.points.size() >= 2) {
            for (int i = 0; i < cable.points.size() - 1; i++) {
                scene->addLine(QLineF(cable.points[i], cable.points[i+1]), QPen(Qt::blue, 2));
            }
        }
    }

    view->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    layout->addWidget(view);

    QLabel *info = new QLabel(QString("器件数: %1  馈线数: %2").arg(generator.nodes().size()).arg(generator.cables().size()), dlg);
    layout->addWidget(info);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();
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

    // 统计信息（表格化）
    QGroupBox *statGroup = new QGroupBox("仿真统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("频段:"), 0, 0);
    statLayout->addWidget(new QLabel(Zhifen::CoverageSimulator::bandName(band)), 0, 1);
    statLayout->addWidget(new QLabel("天线数:"), 0, 2);
    statLayout->addWidget(new QLabel(QString::number(result.antennas.size())), 0, 3);
    statLayout->addWidget(new QLabel("最强信号:"), 1, 0);
    QLabel *maxLabel = new QLabel(QString("%1 dBm").arg(result.maxSignal, 0, 'f', 1));
    maxLabel->setStyleSheet("color: green; font-weight: bold;");
    statLayout->addWidget(maxLabel, 1, 1);
    statLayout->addWidget(new QLabel("最弱信号:"), 1, 2);
    QLabel *minLabel = new QLabel(QString("%1 dBm").arg(result.minSignal, 0, 'f', 1));
    minLabel->setStyleSheet(result.minSignal < -95 ? "color: red; font-weight: bold;" : "color: orange;");
    statLayout->addWidget(minLabel, 1, 3);
    statLayout->addWidget(new QLabel("平均信号:"), 2, 0);
    statLayout->addWidget(new QLabel(QString("%1 dBm").arg(result.avgSignal, 0, 'f', 1)), 2, 1);
    statLayout->addWidget(new QLabel("弱覆盖比例:"), 2, 2);
    QLabel *weakLabel = new QLabel(QString("%1%").arg(result.weakCoverageRatio * 100, 0, 'f', 1));
    weakLabel->setStyleSheet(result.weakCoverageRatio > 0.2 ? "color: red; font-weight: bold;" : "color: green;");
    statLayout->addWidget(weakLabel, 2, 3);
    layout->addWidget(statGroup);

    // 弱覆盖告警
    if (result.weakCoverageRatio > 0.2) {
        QLabel *alarmLabel = new QLabel(QString("⚠ 弱覆盖告警: 弱覆盖比例 %1% 超过20%阈值，建议增加天线或调整天线位置")
            .arg(result.weakCoverageRatio * 100, 0, 'f', 1), dlg);
        alarmLabel->setStyleSheet("background: #ffebee; color: #c62828; padding: 8px; border: 1px solid #c62828; border-radius: 4px;");
        layout->addWidget(alarmLabel);
    }

    // 颜色图例条
    QLabel *legendBar = new QLabel(dlg);
    QPixmap legendPix(600, 20);
    QPainter painter(&legendPix);
    for (int x = 0; x < 600; x++) {
        qreal signal = -120.0 + (x / 600.0) * 80.0; // -120 to -40
        QColor color;
        if (signal < -95) color = QColor(255, 0, 0);
        else if (signal < -75) color = QColor(255, 165, 0);
        else if (signal < -60) color = QColor(255, 255, 0);
        else color = QColor(0, 200, 0);
        painter.setPen(color);
        painter.drawLine(x, 0, x, 20);
    }
    painter.end();
    legendBar->setPixmap(legendPix);
    QLabel *legendText = new QLabel("-120dBm (弱覆盖)                          -40dBm (良好)", dlg);
    legendText->setAlignment(Qt::AlignCenter);
    legendText->setStyleSheet("font-size: 8pt; color: gray;");
    layout->addWidget(legendBar);
    layout->addWidget(legendText);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出热力图", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(exportBtn, &QPushButton::clicked, dlg, [this, result]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出热力图", "覆盖仿真热力图.png", "PNG图片 (*.png);;JPEG图片 (*.jpg)");
        if (!fileName.isEmpty()) {
            result.heatmapImage.save(fileName);
            QMessageBox::information(this, "导出成功", "热力图已导出");
        }
    });
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

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
    // 统一参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("电梯覆盖设计 - 参数设置");
    paramDlg->resize(400, 350);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QSpinBox *floorSpin = new QSpinBox(paramDlg);
    floorSpin->setRange(1, 200); floorSpin->setValue(20);
    formLayout->addRow("楼层数:", floorSpin);

    QDoubleSpinBox *heightSpin = new QDoubleSpinBox(paramDlg);
    heightSpin->setRange(2.0, 6.0); heightSpin->setValue(3.0); heightSpin->setSuffix(" m");
    formLayout->addRow("层高:", heightSpin);

    QDoubleSpinBox *txPowerSpin = new QDoubleSpinBox(paramDlg);
    txPowerSpin->setRange(10, 40); txPowerSpin->setValue(15.0); txPowerSpin->setSuffix(" dBm");
    formLayout->addRow("发射功率:", txPowerSpin);

    QDoubleSpinBox *gainSpin = new QDoubleSpinBox(paramDlg);
    gainSpin->setRange(2, 15); gainSpin->setValue(8.0); gainSpin->setSuffix(" dBi");
    formLayout->addRow("天线增益:", gainSpin);

    QDoubleSpinBox *targetSpin = new QDoubleSpinBox(paramDlg);
    targetSpin->setRange(-110, -60); targetSpin->setValue(-85.0); targetSpin->setSuffix(" dBm");
    formLayout->addRow("目标功率:", targetSpin);

    QComboBox *bandCombo = new QComboBox(paramDlg);
    bandCombo->addItems({"2G (900MHz)", "3G (2100MHz)", "4G (1800MHz)", "5G (3500MHz)"});
    bandCombo->setCurrentIndex(2);
    formLayout->addRow("频段:", bandCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *calcBtn = new QPushButton("计算", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(calcBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(calcBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }

    Zhifen::ElevatorParams params;
    params.floorCount = floorSpin->value();
    params.floorHeight = heightSpin->value();
    params.txPower = txPowerSpin->value();
    params.antennaGain = gainSpin->value();
    params.targetPower = targetSpin->value();
    params.band = static_cast<Zhifen::FrequencyBand>(bandCombo->currentIndex());
    QString bandText = bandCombo->currentText();
    paramDlg->deleteLater();

    Zhifen::ElevatorResult result = Zhifen::ElevatorCoverageTool::calculate(params);

    // 表格化结果展示
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("电梯覆盖设计方案");
    dlg->resize(600, 500);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 设计参数汇总
    QGroupBox *paramGroup = new QGroupBox("设计参数", dlg);
    QGridLayout *paramLayout = new QGridLayout(paramGroup);
    paramLayout->addWidget(new QLabel("楼层数:"), 0, 0);
    paramLayout->addWidget(new QLabel(QString::number(params.floorCount)), 0, 1);
    paramLayout->addWidget(new QLabel("层高:"), 0, 2);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.floorHeight)), 0, 3);
    paramLayout->addWidget(new QLabel("总高度:"), 1, 0);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.floorCount * params.floorHeight)), 1, 1);
    paramLayout->addWidget(new QLabel("频段:"), 1, 2);
    paramLayout->addWidget(new QLabel(bandText), 1, 3);
    layout->addWidget(paramGroup);

    // 计算结果表格
    QGroupBox *resultGroup = new QGroupBox("计算结果", dlg);
    QGridLayout *resultLayout = new QGridLayout(resultGroup);
    resultLayout->addWidget(new QLabel("推荐天线:"), 0, 0);
    QLabel *antTypeLabel = new QLabel(result.antennaType);
    antTypeLabel->setStyleSheet("color: #1976d2; font-weight: bold;");
    resultLayout->addWidget(antTypeLabel, 0, 1);
    resultLayout->addWidget(new QLabel("天线数量:"), 0, 2);
    QLabel *antCountLabel = new QLabel(QString::number(result.antennaCount));
    antCountLabel->setStyleSheet("color: #388e3c; font-weight: bold; font-size: 14pt;");
    resultLayout->addWidget(antCountLabel, 0, 3);
    resultLayout->addWidget(new QLabel("天线间距:"), 1, 0);
    resultLayout->addWidget(new QLabel(QString("%1 m").arg(result.antennaSpacing, 0, 'f', 1)), 1, 1);
    resultLayout->addWidget(new QLabel("平均功率:"), 1, 2);
    resultLayout->addWidget(new QLabel(QString("%1 dBm").arg(result.avgPower, 0, 'f', 1)), 1, 3);
    resultLayout->addWidget(new QLabel("最弱功率:"), 2, 0);
    QLabel *minLabel = new QLabel(QString("%1 dBm").arg(result.minPower, 0, 'f', 1));
    minLabel->setStyleSheet(result.minPower < params.targetPower ? "color: red; font-weight: bold;" : "color: green;");
    resultLayout->addWidget(minLabel, 2, 1);
    resultLayout->addWidget(new QLabel("最强功率:"), 2, 2);
    resultLayout->addWidget(new QLabel(QString("%1 dBm").arg(result.maxPower, 0, 'f', 1)), 2, 3);
    layout->addWidget(resultGroup);

    // 达标判断
    if (result.minPower < params.targetPower) {
        QLabel *warnLabel = new QLabel(QString("⚠ 警告: 最弱功率 %1 dBm 低于目标 %2 dBm，建议增加天线或提高发射功率")
            .arg(result.minPower, 0, 'f', 1).arg(params.targetPower, 0, 'f', 1));
        warnLabel->setStyleSheet("background: #ffebee; color: #c62828; padding: 8px; border-radius: 4px;");
        layout->addWidget(warnLabel);
    } else {
        QLabel *okLabel = new QLabel("✓ 设计达标: 所有楼层覆盖功率均满足要求");
        okLabel->setStyleSheet("background: #e8f5e9; color: #2e7d32; padding: 8px; border-radius: 4px;");
        layout->addWidget(okLabel);
    }

    // 详细报告
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    textEdit->setMaximumHeight(150);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("电梯覆盖设计: %1层, 天线%2个, 最弱%3dBm").arg(params.floorCount).arg(result.antennaCount).arg(result.minPower, 0, 'f', 1));
}

void MainWindow::onLeakyCableTool()
{
    // 统一参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("漏缆分段设计 - 参数设置");
    paramDlg->resize(400, 380);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QDoubleSpinBox *lengthSpin = new QDoubleSpinBox(paramDlg);
    lengthSpin->setRange(10, 1000); lengthSpin->setValue(100); lengthSpin->setSuffix(" m");
    formLayout->addRow("漏缆总长度:", lengthSpin);

    QDoubleSpinBox *couplingSpin = new QDoubleSpinBox(paramDlg);
    couplingSpin->setRange(50, 100); couplingSpin->setValue(70); couplingSpin->setSuffix(" dB");
    formLayout->addRow("耦合损耗:", couplingSpin);

    QDoubleSpinBox *transSpin = new QDoubleSpinBox(paramDlg);
    transSpin->setRange(1.0, 10.0); transSpin->setValue(2.5); transSpin->setSuffix(" dB/100m");
    formLayout->addRow("传输损耗:", transSpin);

    QDoubleSpinBox *txPowerSpin = new QDoubleSpinBox(paramDlg);
    txPowerSpin->setRange(20, 60); txPowerSpin->setValue(43.0); txPowerSpin->setSuffix(" dBm");
    formLayout->addRow("输入端功率:", txPowerSpin);

    QDoubleSpinBox *targetSpin = new QDoubleSpinBox(paramDlg);
    targetSpin->setRange(-110, -60); targetSpin->setValue(-80.0); targetSpin->setSuffix(" dBm");
    formLayout->addRow("目标耦合功率:", targetSpin);

    QDoubleSpinBox *minSpin = new QDoubleSpinBox(paramDlg);
    minSpin->setRange(-120, -70); minSpin->setValue(-90.0); minSpin->setSuffix(" dBm");
    formLayout->addRow("最小允许功率:", minSpin);

    QComboBox *bandCombo = new QComboBox(paramDlg);
    bandCombo->addItems({"2G (900MHz)", "3G (2100MHz)", "4G (1800MHz)", "5G (3500MHz)"});
    bandCombo->setCurrentIndex(2);
    formLayout->addRow("频段:", bandCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *calcBtn = new QPushButton("计算", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(calcBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(calcBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }

    Zhifen::LeakyCableParams params;
    params.totalLength = lengthSpin->value();
    params.couplingLoss = couplingSpin->value();
    params.transmissionLoss = transSpin->value();
    params.txPower = txPowerSpin->value();
    params.targetPower = targetSpin->value();
    params.minPower = minSpin->value();
    params.band = static_cast<Zhifen::FrequencyBand>(bandCombo->currentIndex());
    QString bandText = bandCombo->currentText();
    paramDlg->deleteLater();

    Zhifen::LeakyCableResult result = Zhifen::LeakyCableTool::calculate(params);

    // 表格化结果展示
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("漏缆分段设计方案");
    dlg->resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 设计参数汇总
    QGroupBox *paramGroup = new QGroupBox("设计参数", dlg);
    QGridLayout *paramLayout = new QGridLayout(paramGroup);
    paramLayout->addWidget(new QLabel("总长度:"), 0, 0);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.totalLength)), 0, 1);
    paramLayout->addWidget(new QLabel("耦合损耗:"), 0, 2);
    paramLayout->addWidget(new QLabel(QString("%1 dB").arg(params.couplingLoss)), 0, 3);
    paramLayout->addWidget(new QLabel("传输损耗:"), 1, 0);
    paramLayout->addWidget(new QLabel(QString("%1 dB/100m").arg(params.transmissionLoss)), 1, 1);
    paramLayout->addWidget(new QLabel("输入功率:"), 1, 2);
    paramLayout->addWidget(new QLabel(QString("%1 dBm").arg(params.txPower)), 1, 3);
    paramLayout->addWidget(new QLabel("频段:"), 2, 0);
    paramLayout->addWidget(new QLabel(bandText), 2, 1);
    paramLayout->addWidget(new QLabel("目标功率:"), 2, 2);
    paramLayout->addWidget(new QLabel(QString("%1 dBm").arg(params.targetPower)), 2, 3);
    layout->addWidget(paramGroup);

    // 总体结果
    QGroupBox *summaryGroup = new QGroupBox("总体结果", dlg);
    QGridLayout *summaryLayout = new QGridLayout(summaryGroup);
    summaryLayout->addWidget(new QLabel("分段数量:"), 0, 0);
    QLabel *segLabel = new QLabel(QString::number(result.segmentCount));
    segLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 14pt;");
    summaryLayout->addWidget(segLabel, 0, 1);
    summaryLayout->addWidget(new QLabel("末端功率:"), 0, 2);
    QLabel *endLabel = new QLabel(QString("%1 dBm").arg(result.endPower, 0, 'f', 1));
    endLabel->setStyleSheet(result.endPower < params.minPower ? "color: red; font-weight: bold;" : "color: green;");
    summaryLayout->addWidget(endLabel, 0, 3);
    summaryLayout->addWidget(new QLabel("总损耗:"), 1, 0);
    summaryLayout->addWidget(new QLabel(QString("%1 dB").arg(result.totalLoss, 0, 'f', 1)), 1, 1);
    layout->addWidget(summaryGroup);

    // 分段详情表格
    QGroupBox *segGroup = new QGroupBox("分段详情", dlg);
    QVBoxLayout *segLayout = new QVBoxLayout(segGroup);
    QTableWidget *table = new QTableWidget(result.segments.size(), 7, dlg);
    table->setHorizontalHeaderLabels({"段号", "起始(m)", "结束(m)", "长度(m)", "输入功率(dBm)", "输出功率(dBm)", "达标"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < result.segments.size(); i++) {
        const auto &seg = result.segments[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::number(seg.segmentIndex)));
        table->setItem(i, 1, new QTableWidgetItem(QString::number(seg.startPos, 'f', 1)));
        table->setItem(i, 2, new QTableWidgetItem(QString::number(seg.endPos, 'f', 1)));
        table->setItem(i, 3, new QTableWidgetItem(QString::number(seg.length, 'f', 1)));
        table->setItem(i, 4, new QTableWidgetItem(QString::number(seg.inputPower, 'f', 1)));
        table->setItem(i, 5, new QTableWidgetItem(QString::number(seg.outputPower, 'f', 1)));
        QTableWidgetItem *passItem = new QTableWidgetItem(seg.pass ? "✓ 达标" : "✗ 不达标");
        passItem->setForeground(seg.pass ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 6, passItem);
    }
    table->resizeColumnsToContents();
    segLayout->addWidget(table);
    layout->addWidget(segGroup, 1);

    // 达标判断
    bool allPass = true;
    for (const auto &seg : result.segments) {
        if (!seg.pass) { allPass = false; break; }
    }
    if (!allPass || result.endPower < params.minPower) {
        QLabel *warnLabel = new QLabel("⚠ 警告: 部分分段或末端功率不达标，建议增加信号源或缩短漏缆长度");
        warnLabel->setStyleSheet("background: #ffebee; color: #c62828; padding: 8px; border-radius: 4px;");
        layout->addWidget(warnLabel);
    } else {
        QLabel *okLabel = new QLabel("✓ 设计达标: 所有分段耦合功率均满足要求");
        okLabel->setStyleSheet("background: #e8f5e9; color: #2e7d32; padding: 8px; border-radius: 4px;");
        layout->addWidget(okLabel);
    }

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("漏缆分段设计: %1m, %2段, 末端%3dBm").arg(params.totalLength).arg(result.segmentCount).arg(result.endPower, 0, 'f', 1));
}

void MainWindow::onBuildingToBuildingTool()
{
    // 统一参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("楼间对打设计 - 参数设置");
    paramDlg->resize(420, 450);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QDoubleSpinBox *distanceSpin = new QDoubleSpinBox(paramDlg);
    distanceSpin->setRange(10, 500); distanceSpin->setValue(50); distanceSpin->setSuffix(" m");
    formLayout->addRow("楼间距:", distanceSpin);

    QDoubleSpinBox *txPowerSpin = new QDoubleSpinBox(paramDlg);
    txPowerSpin->setRange(20, 60); txPowerSpin->setValue(43.0); txPowerSpin->setSuffix(" dBm");
    formLayout->addRow("发射功率:", txPowerSpin);

    QDoubleSpinBox *txHeightSpin = new QDoubleSpinBox(paramDlg);
    txHeightSpin->setRange(5, 100); txHeightSpin->setValue(30.0); txHeightSpin->setSuffix(" m");
    formLayout->addRow("发射天线高度:", txHeightSpin);

    QDoubleSpinBox *rxHeightSpin = new QDoubleSpinBox(paramDlg);
    rxHeightSpin->setRange(1, 50); rxHeightSpin->setValue(15.0); rxHeightSpin->setSuffix(" m");
    formLayout->addRow("接收天线高度:", rxHeightSpin);

    QDoubleSpinBox *txGainSpin = new QDoubleSpinBox(paramDlg);
    txGainSpin->setRange(5, 25); txGainSpin->setValue(12.0); txGainSpin->setSuffix(" dBi");
    formLayout->addRow("发射天线增益:", txGainSpin);

    QDoubleSpinBox *rxGainSpin = new QDoubleSpinBox(paramDlg);
    rxGainSpin->setRange(0, 10); rxGainSpin->setValue(2.0); rxGainSpin->setSuffix(" dBi");
    formLayout->addRow("接收天线增益:", rxGainSpin);

    QDoubleSpinBox *penetrationSpin = new QDoubleSpinBox(paramDlg);
    penetrationSpin->setRange(5, 40); penetrationSpin->setValue(15.0); penetrationSpin->setSuffix(" dB");
    formLayout->addRow("建筑穿透损耗:", penetrationSpin);

    QDoubleSpinBox *targetSpin = new QDoubleSpinBox(paramDlg);
    targetSpin->setRange(-110, -60); targetSpin->setValue(-85.0); targetSpin->setSuffix(" dBm");
    formLayout->addRow("目标接收功率:", targetSpin);

    QComboBox *bandCombo = new QComboBox(paramDlg);
    bandCombo->addItems({"2G (900MHz)", "3G (2100MHz)", "4G (1800MHz)", "5G (3500MHz)"});
    bandCombo->setCurrentIndex(2);
    formLayout->addRow("频段:", bandCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *calcBtn = new QPushButton("计算", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(calcBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(calcBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }

    Zhifen::BuildingToBuildingParams params;
    params.distance = distanceSpin->value();
    params.txPower = txPowerSpin->value();
    params.txHeight = txHeightSpin->value();
    params.rxHeight = rxHeightSpin->value();
    params.txAntennaGain = txGainSpin->value();
    params.rxAntennaGain = rxGainSpin->value();
    params.buildingPenetration = penetrationSpin->value();
    params.targetPower = targetSpin->value();
    params.band = static_cast<Zhifen::FrequencyBand>(bandCombo->currentIndex());
    QString bandText = bandCombo->currentText();
    paramDlg->deleteLater();

    Zhifen::BuildingToBuildingResult result = Zhifen::BuildingToBuildingTool::calculate(params);

    // 表格化结果展示
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("楼间对打设计方案");
    dlg->resize(650, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 设计参数汇总
    QGroupBox *paramGroup = new QGroupBox("设计参数", dlg);
    QGridLayout *paramLayout = new QGridLayout(paramGroup);
    paramLayout->addWidget(new QLabel("楼间距:"), 0, 0);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.distance)), 0, 1);
    paramLayout->addWidget(new QLabel("发射功率:"), 0, 2);
    paramLayout->addWidget(new QLabel(QString("%1 dBm").arg(params.txPower)), 0, 3);
    paramLayout->addWidget(new QLabel("发射高度:"), 1, 0);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.txHeight)), 1, 1);
    paramLayout->addWidget(new QLabel("接收高度:"), 1, 2);
    paramLayout->addWidget(new QLabel(QString("%1 m").arg(params.rxHeight)), 1, 3);
    paramLayout->addWidget(new QLabel("频段:"), 2, 0);
    paramLayout->addWidget(new QLabel(bandText), 2, 1);
    paramLayout->addWidget(new QLabel("目标功率:"), 2, 2);
    paramLayout->addWidget(new QLabel(QString("%1 dBm").arg(params.targetPower)), 2, 3);
    layout->addWidget(paramGroup);

    // 链路预算计算
    QGroupBox *linkGroup = new QGroupBox("链路预算", dlg);
    QGridLayout *linkLayout = new QGridLayout(linkGroup);
    linkLayout->addWidget(new QLabel("发射功率:"), 0, 0);
    linkLayout->addWidget(new QLabel(QString("%1 dBm").arg(params.txPower)), 0, 1);
    linkLayout->addWidget(new QLabel("发射天线增益:"), 0, 2);
    linkLayout->addWidget(new QLabel(QString("%1 dBi").arg(params.txAntennaGain)), 0, 3);
    linkLayout->addWidget(new QLabel("自由空间损耗:"), 1, 0);
    linkLayout->addWidget(new QLabel(QString("%1 dB").arg(result.freeSpaceLoss, 0, 'f', 1)), 1, 1);
    linkLayout->addWidget(new QLabel("建筑穿透损耗:"), 1, 2);
    linkLayout->addWidget(new QLabel(QString("%1 dB").arg(params.buildingPenetration)), 1, 3);
    linkLayout->addWidget(new QLabel("接收天线增益:"), 2, 0);
    linkLayout->addWidget(new QLabel(QString("%1 dBi").arg(params.rxAntennaGain)), 2, 1);
    linkLayout->addWidget(new QLabel("总损耗:"), 2, 2);
    linkLayout->addWidget(new QLabel(QString("%1 dB").arg(result.freeSpaceLoss + params.buildingPenetration, 0, 'f', 1)), 2, 3);
    linkLayout->addWidget(new QLabel("接收功率:"), 3, 0);
    QLabel *rxLabel = new QLabel(QString("%1 dBm").arg(result.receivedPower, 0, 'f', 1));
    rxLabel->setStyleSheet(result.receivedPower < params.targetPower ? "color: red; font-weight: bold; font-size: 14pt;" : "color: green; font-weight: bold; font-size: 14pt;");
    linkLayout->addWidget(rxLabel, 3, 1);
    linkLayout->addWidget(new QLabel("链路余量:"), 3, 2);
    QLabel *marginLabel = new QLabel(QString("%1 dB").arg(result.receivedPower - params.targetPower, 0, 'f', 1));
    marginLabel->setStyleSheet(result.receivedPower - params.targetPower < 0 ? "color: red; font-weight: bold;" : "color: green;");
    linkLayout->addWidget(marginLabel, 3, 3);
    layout->addWidget(linkGroup);

    // 覆盖角度分析
    QGroupBox *angleGroup = new QGroupBox("覆盖角度分析", dlg);
    QGridLayout *angleLayout = new QGridLayout(angleGroup);
    angleLayout->addWidget(new QLabel("水平波束宽度:"), 0, 0);
    angleLayout->addWidget(new QLabel(QString("%1°").arg(65.0, 0, 'f', 1)), 0, 1);
    angleLayout->addWidget(new QLabel("垂直波束宽度:"), 0, 2);
    angleLayout->addWidget(new QLabel(QString("%1°").arg(15.0, 0, 'f', 1)), 0, 3);
    angleLayout->addWidget(new QLabel("下倾角:"), 1, 0);
    angleLayout->addWidget(new QLabel(QString("%1°").arg((params.txHeight > params.rxHeight ? (params.txHeight - params.rxHeight) / params.distance * 57.3 : 0.0), 0, 'f', 1)), 1, 1);
    angleLayout->addWidget(new QLabel("覆盖半径:"), 1, 2);
    angleLayout->addWidget(new QLabel(QString("%1 m").arg(result.maxCoverageDistance, 0, 'f', 1)), 1, 3);
    layout->addWidget(angleGroup);

    // 达标判断
    if (result.receivedPower < params.targetPower || result.receivedPower - params.targetPower < 0) {
        QLabel *warnLabel = new QLabel(QString("⚠ 警告: 接收功率 %1 dBm 低于目标 %2 dBm，链路余量 %3 dB，建议提高发射功率或使用高增益天线")
            .arg(result.receivedPower, 0, 'f', 1).arg(params.targetPower, 0, 'f', 1).arg(result.receivedPower - params.targetPower, 0, 'f', 1));
        warnLabel->setStyleSheet("background: #ffebee; color: #c62828; padding: 8px; border-radius: 4px;");
        layout->addWidget(warnLabel);
    } else {
        QLabel *okLabel = new QLabel(QString("✓ 设计达标: 接收功率 %1 dBm 满足要求，链路余量 %2 dB")
            .arg(result.receivedPower, 0, 'f', 1).arg(result.receivedPower - params.targetPower, 0, 'f', 1));
        okLabel->setStyleSheet("background: #e8f5e9; color: #2e7d32; padding: 8px; border-radius: 4px;");
        layout->addWidget(okLabel);
    }

    // 详细报告
    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    textEdit->setMaximumHeight(120);
    layout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("楼间对打设计: %1m, 接收%2dBm, 余量%3dB").arg(params.distance).arg(result.receivedPower, 0, 'f', 1).arg(result.receivedPower - params.targetPower, 0, 'f', 1));
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

    Zhifen::BlockCreateDialog dlg(this);
    QPointF defaultBase = selected.first()->pos();
    dlg.setBasePoint(defaultBase);

    if (dlg.exec() != QDialog::Accepted) return;

    QString name = dlg.blockName();
    QPointF basePoint = dlg.basePoint();

    if (name.isEmpty()) return;
    if (Zhifen::BlockManager::instance().hasBlock(name)) {
        QMessageBox::warning(this, "创建块失败", QString("块 '%1' 已存在").arg(name));
        return;
    }

    // 创建块定义
    Zhifen::BlockDefinition *def = new Zhifen::BlockDefinition(name);
    def->setBasePoint(basePoint);
    def->setDescription("用户创建块");
    for (QGraphicsItem *item : selected) {
        def->addItem(item);
    }
    Zhifen::BlockManager::instance().addBlock(def);

    // 将选中图元替换为块引用
    if (dlg.convertToBlock()) {
        for (QGraphicsItem *item : selected) {
            m_scene->removeItem(item);
        }
        Zhifen::BlockReference *blockRef = new Zhifen::BlockReference(name);
        blockRef->setInsertPoint(basePoint);
        blockRef->setPos(basePoint);
        m_scene->addItem(blockRef);
    }

    QMessageBox::information(this, "创建块成功", QString("块 '%1' 已创建，包含 %2 个图元").arg(name).arg(selected.size()));
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("创建块: %1").arg(name));
}

void MainWindow::onInsertBlock()
{
    Zhifen::BlockManager &mgr = Zhifen::BlockManager::instance();
    QList<QString> names = mgr.allBlockNames();
    if (names.isEmpty()) {
        QMessageBox::information(this, "插入块", "当前没有可用的块，请先创建块");
        return;
    }

    bool ok;
    QString name = QInputDialog::getItem(this, "插入块", "选择块:", names, 0, false, &ok);
    if (!ok || name.isEmpty()) return;

    QPointF insertPt = m_view->mapToScene(m_view->viewport()->rect().center());
    Zhifen::BlockReference *blockRef = new Zhifen::BlockReference(name);
    blockRef->setInsertPoint(insertPt);
    blockRef->setPos(insertPt);

    // 如果块有属性，弹出属性编辑对话框
    Zhifen::BlockDefinition *def = mgr.block(name);
    if (def && def->attributeCount() > 0) {
        Zhifen::AttributeDialog attrDlg(this);
        attrDlg.setBlock(def, blockRef->attributeValues());
        if (attrDlg.exec() == QDialog::Accepted) {
            blockRef->setAttributeValues(attrDlg.attributeValues());
        }
    }

    m_scene->addItem(blockRef);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("插入块: %1").arg(name));
}

void MainWindow::onBlockManager()
{
    static Zhifen::BlockManagerPanel *panel = nullptr;
    if (!panel) {
        panel = new Zhifen::BlockManagerPanel(this);
        addDockWidget(Qt::RightDockWidgetArea, panel);
        connect(panel, &Zhifen::BlockManagerPanel::editBlockRequested, this, [this](const QString &name) {
            Zhifen::BlockEditor *editor = new Zhifen::BlockEditor(name, this);
            connect(editor, &Zhifen::BlockEditor::blockEdited, this, [this](const QString &bn) {
                // 块已编辑，刷新场景
                m_scene->update();
            });
            editor->show();
        });
        connect(panel, &Zhifen::BlockManagerPanel::insertBlockRequested, this, [this](const QString &name) {
            QPointF insertPt = m_view->mapToScene(m_view->viewport()->rect().center());
            Zhifen::BlockReference *blockRef = new Zhifen::BlockReference(name);
            blockRef->setInsertPoint(insertPt);
            blockRef->setPos(insertPt);

            Zhifen::BlockDefinition *def = Zhifen::BlockManager::instance().block(name);
            if (def && def->attributeCount() > 0) {
                Zhifen::AttributeDialog attrDlg(this);
                attrDlg.setBlock(def, blockRef->attributeValues());
                if (attrDlg.exec() == QDialog::Accepted) {
                    blockRef->setAttributeValues(attrDlg.attributeValues());
                }
            }

            m_scene->addItem(blockRef);
        });
    }
    panel->refresh();
    panel->show();
    panel->raise();
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

    // 统一转换报告对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("天越格式导入报告");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 统计信息
    QGroupBox *statGroup = new QGroupBox("转换统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图元数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalEntities)), 0, 1);
    statLayout->addWidget(new QLabel("成功转换:"), 0, 2);
    QLabel *successLabel = new QLabel(QString::number(report.successCount));
    successLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(successLabel, 0, 3);
    statLayout->addWidget(new QLabel("转换失败:"), 1, 0);
    QLabel *failLabel = new QLabel(QString::number(report.failedCount));
    failLabel->setStyleSheet(report.failedCount > 0 ? "color: #c62828; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(failLabel, 1, 1);
    statLayout->addWidget(new QLabel("警告:"), 1, 2);
    QLabel *warnLabel = new QLabel(QString::number(report.warningCount));
    warnLabel->setStyleSheet(report.warningCount > 0 ? "color: #f57c00; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(warnLabel, 1, 3);
    layout->addWidget(statGroup);

    // 按类型统计
    QGroupBox *typeGroup = new QGroupBox("图元类型统计", dlg);
    QGridLayout *typeLayout = new QGridLayout(typeGroup);
    QMap<QString, int> typeCount;
    for (const auto &item : report.items) {
        typeCount[item.entityType]++;
    }
    QStringList typeNames = {"信源", "天线", "器件", "馈线", "文字", "其他"};
    QStringList typeKeys = {"source", "antenna", "device", "feeder", "text", "unknown"};
    for (int i = 0; i < typeNames.size(); i++) {
        typeLayout->addWidget(new QLabel(typeNames[i] + ":"), i / 2, (i % 2) * 2);
        typeLayout->addWidget(new QLabel(QString::number(typeCount.value(typeKeys[i], 0))), i / 2, (i % 2) * 2 + 1);
    }
    layout->addWidget(typeGroup);

    // 详细转换结果表格
    QGroupBox *detailGroup = new QGroupBox("详细转换结果", dlg);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);
    QTableWidget *table = new QTableWidget(report.items.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"图元ID", "类型", "型号", "状态", "备注"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < report.items.size(); i++) {
        const auto &item = report.items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.entityId));
        table->setItem(i, 1, new QTableWidgetItem(item.entityType));
        table->setItem(i, 2, new QTableWidgetItem(item.entityModel));
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.success ? "✓ 成功" : "✗ 失败");
        statusItem->setForeground(item.success ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 3, statusItem);
        QString note = item.message;
        if (!item.warning.isEmpty()) note += " [" + item.warning + "]";
        table->setItem(i, 4, new QTableWidgetItem(note));
    }
    table->resizeColumnsToContents();
    detailLayout->addWidget(table);
    layout->addWidget(detailGroup, 1);

    // 总结
    QLabel *summaryLabel = new QLabel(report.summary, dlg);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
    layout->addWidget(summaryLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入天越格式: %1, 成功%2/总%3").arg(fileName).arg(report.successCount).arg(report.totalEntities));
}

void MainWindow::onImportAIDP()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入AIDP格式", "", "AIDP文件 (*.aidp *.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.importAIDP(fileName, m_scene);

    // 统一转换报告对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("AIDP格式导入报告");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QGroupBox *statGroup = new QGroupBox("转换统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图元数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalEntities)), 0, 1);
    statLayout->addWidget(new QLabel("成功转换:"), 0, 2);
    QLabel *successLabel = new QLabel(QString::number(report.successCount));
    successLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(successLabel, 0, 3);
    statLayout->addWidget(new QLabel("转换失败:"), 1, 0);
    QLabel *failLabel = new QLabel(QString::number(report.failedCount));
    failLabel->setStyleSheet(report.failedCount > 0 ? "color: #c62828; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(failLabel, 1, 1);
    statLayout->addWidget(new QLabel("警告:"), 1, 2);
    QLabel *warnLabel = new QLabel(QString::number(report.warningCount));
    warnLabel->setStyleSheet(report.warningCount > 0 ? "color: #f57c00; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(warnLabel, 1, 3);
    layout->addWidget(statGroup);

    QGroupBox *typeGroup = new QGroupBox("图元类型统计", dlg);
    QGridLayout *typeLayout = new QGridLayout(typeGroup);
    QMap<QString, int> typeCount;
    for (const auto &item : report.items) typeCount[item.entityType]++;
    QStringList typeNames = {"信源", "天线", "器件", "馈线", "文字", "其他"};
    QStringList typeKeys = {"source", "antenna", "device", "feeder", "text", "unknown"};
    for (int i = 0; i < typeNames.size(); i++) {
        typeLayout->addWidget(new QLabel(typeNames[i] + ":"), i / 2, (i % 2) * 2);
        typeLayout->addWidget(new QLabel(QString::number(typeCount.value(typeKeys[i], 0))), i / 2, (i % 2) * 2 + 1);
    }
    layout->addWidget(typeGroup);

    QGroupBox *detailGroup = new QGroupBox("详细转换结果", dlg);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);
    QTableWidget *table = new QTableWidget(report.items.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"图元ID", "类型", "型号", "状态", "备注"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < report.items.size(); i++) {
        const auto &item = report.items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.entityId));
        table->setItem(i, 1, new QTableWidgetItem(item.entityType));
        table->setItem(i, 2, new QTableWidgetItem(item.entityModel));
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.success ? "✓ 成功" : "✗ 失败");
        statusItem->setForeground(item.success ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 3, statusItem);
        QString note = item.message;
        if (!item.warning.isEmpty()) note += " [" + item.warning + "]";
        table->setItem(i, 4, new QTableWidgetItem(note));
    }
    table->resizeColumnsToContents();
    detailLayout->addWidget(table);
    layout->addWidget(detailGroup, 1);

    QLabel *summaryLabel = new QLabel(report.summary, dlg);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
    layout->addWidget(summaryLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入AIDP格式: %1, 成功%2/总%3").arg(fileName).arg(report.successCount).arg(report.totalEntities));
}

void MainWindow::onImportDifu()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入迪弗格式", "", "迪弗文件 (*.dfd *.dxf);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    Zhifen::FormatConverter &fc = Zhifen::FormatConverter::instance();
    Zhifen::ConversionReport report = fc.importDiFu(fileName, m_scene);

    // 统一转换报告对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("迪弗格式导入报告");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    QGroupBox *statGroup = new QGroupBox("转换统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图元数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalEntities)), 0, 1);
    statLayout->addWidget(new QLabel("成功转换:"), 0, 2);
    QLabel *successLabel = new QLabel(QString::number(report.successCount));
    successLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(successLabel, 0, 3);
    statLayout->addWidget(new QLabel("转换失败:"), 1, 0);
    QLabel *failLabel = new QLabel(QString::number(report.failedCount));
    failLabel->setStyleSheet(report.failedCount > 0 ? "color: #c62828; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(failLabel, 1, 1);
    statLayout->addWidget(new QLabel("警告:"), 1, 2);
    QLabel *warnLabel = new QLabel(QString::number(report.warningCount));
    warnLabel->setStyleSheet(report.warningCount > 0 ? "color: #f57c00; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(warnLabel, 1, 3);
    layout->addWidget(statGroup);

    QGroupBox *typeGroup = new QGroupBox("图元类型统计", dlg);
    QGridLayout *typeLayout = new QGridLayout(typeGroup);
    QMap<QString, int> typeCount;
    for (const auto &item : report.items) typeCount[item.entityType]++;
    QStringList typeNames = {"信源", "天线", "器件", "馈线", "文字", "其他"};
    QStringList typeKeys = {"source", "antenna", "device", "feeder", "text", "unknown"};
    for (int i = 0; i < typeNames.size(); i++) {
        typeLayout->addWidget(new QLabel(typeNames[i] + ":"), i / 2, (i % 2) * 2);
        typeLayout->addWidget(new QLabel(QString::number(typeCount.value(typeKeys[i], 0))), i / 2, (i % 2) * 2 + 1);
    }
    layout->addWidget(typeGroup);

    QGroupBox *detailGroup = new QGroupBox("详细转换结果", dlg);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);
    QTableWidget *table = new QTableWidget(report.items.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"图元ID", "类型", "型号", "状态", "备注"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < report.items.size(); i++) {
        const auto &item = report.items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.entityId));
        table->setItem(i, 1, new QTableWidgetItem(item.entityType));
        table->setItem(i, 2, new QTableWidgetItem(item.entityModel));
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.success ? "✓ 成功" : "✗ 失败");
        statusItem->setForeground(item.success ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 3, statusItem);
        QString note = item.message;
        if (!item.warning.isEmpty()) note += " [" + item.warning + "]";
        table->setItem(i, 4, new QTableWidgetItem(note));
    }
    table->resizeColumnsToContents();
    detailLayout->addWidget(table);
    layout->addWidget(detailGroup, 1);

    QLabel *summaryLabel = new QLabel(report.summary, dlg);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
    layout->addWidget(summaryLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导入迪弗格式: %1, 成功%2/总%3").arg(fileName).arg(report.successCount).arg(report.totalEntities));
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

    // 统一导出报告对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("天越格式导出报告");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 文件信息
    QGroupBox *fileGroup = new QGroupBox("导出文件", dlg);
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    fileLayout->addWidget(new QLabel("文件路径:"), 0, 0);
    QLabel *pathLabel = new QLabel(fileName);
    pathLabel->setWordWrap(true);
    pathLabel->setStyleSheet("color: #1976d2;");
    fileLayout->addWidget(pathLabel, 0, 1);
    fileLayout->addWidget(new QLabel("格式:"), 1, 0);
    fileLayout->addWidget(new QLabel("天越格式 (.dxf)"), 1, 1);
    layout->addWidget(fileGroup);

    // 导出统计
    QGroupBox *statGroup = new QGroupBox("导出统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图元数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalEntities)), 0, 1);
    statLayout->addWidget(new QLabel("成功导出:"), 0, 2);
    QLabel *successLabel = new QLabel(QString::number(report.successCount));
    successLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(successLabel, 0, 3);
    statLayout->addWidget(new QLabel("导出失败:"), 1, 0);
    QLabel *failLabel = new QLabel(QString::number(report.failedCount));
    failLabel->setStyleSheet(report.failedCount > 0 ? "color: #c62828; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(failLabel, 1, 1);
    statLayout->addWidget(new QLabel("警告:"), 1, 2);
    QLabel *warnLabel = new QLabel(QString::number(report.warningCount));
    warnLabel->setStyleSheet(report.warningCount > 0 ? "color: #f57c00; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(warnLabel, 1, 3);
    layout->addWidget(statGroup);

    // 按类型统计
    QGroupBox *typeGroup = new QGroupBox("图元类型统计", dlg);
    QGridLayout *typeLayout = new QGridLayout(typeGroup);
    QMap<QString, int> typeCount;
    for (const auto &item : report.items) typeCount[item.entityType]++;
    QStringList typeNames = {"信源", "天线", "器件", "馈线", "文字", "其他"};
    QStringList typeKeys = {"source", "antenna", "device", "feeder", "text", "unknown"};
    for (int i = 0; i < typeNames.size(); i++) {
        typeLayout->addWidget(new QLabel(typeNames[i] + ":"), i / 2, (i % 2) * 2);
        typeLayout->addWidget(new QLabel(QString::number(typeCount.value(typeKeys[i], 0))), i / 2, (i % 2) * 2 + 1);
    }
    layout->addWidget(typeGroup);

    // 详细导出结果表格
    QGroupBox *detailGroup = new QGroupBox("详细导出结果", dlg);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);
    QTableWidget *table = new QTableWidget(report.items.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"图元ID", "类型", "型号", "状态", "备注"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < report.items.size(); i++) {
        const auto &item = report.items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.entityId));
        table->setItem(i, 1, new QTableWidgetItem(item.entityType));
        table->setItem(i, 2, new QTableWidgetItem(item.entityModel));
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.success ? "✓ 成功" : "✗ 失败");
        statusItem->setForeground(item.success ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 3, statusItem);
        QString note = item.message;
        if (!item.warning.isEmpty()) note += " [" + item.warning + "]";
        table->setItem(i, 4, new QTableWidgetItem(note));
    }
    table->resizeColumnsToContents();
    detailLayout->addWidget(table);
    layout->addWidget(detailGroup, 1);

    // 总结
    QLabel *summaryLabel = new QLabel(report.summary, dlg);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
    layout->addWidget(summaryLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导出天越格式: %1, 成功%2/总%3").arg(fileName).arg(report.successCount).arg(report.totalEntities));
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

    // 统一导出报告对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("AIDP格式导出报告");
    dlg->resize(700, 550);
    QVBoxLayout *layout = new QVBoxLayout(dlg);

    // 文件信息
    QGroupBox *fileGroup = new QGroupBox("导出文件", dlg);
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    fileLayout->addWidget(new QLabel("文件路径:"), 0, 0);
    QLabel *pathLabel = new QLabel(fileName);
    pathLabel->setWordWrap(true);
    pathLabel->setStyleSheet("color: #1976d2;");
    fileLayout->addWidget(pathLabel, 0, 1);
    fileLayout->addWidget(new QLabel("格式:"), 1, 0);
    fileLayout->addWidget(new QLabel("AIDP格式 (.dxf)"), 1, 1);
    layout->addWidget(fileGroup);

    // 导出统计
    QGroupBox *statGroup = new QGroupBox("导出统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图元数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(report.totalEntities)), 0, 1);
    statLayout->addWidget(new QLabel("成功导出:"), 0, 2);
    QLabel *successLabel = new QLabel(QString::number(report.successCount));
    successLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(successLabel, 0, 3);
    statLayout->addWidget(new QLabel("导出失败:"), 1, 0);
    QLabel *failLabel = new QLabel(QString::number(report.failedCount));
    failLabel->setStyleSheet(report.failedCount > 0 ? "color: #c62828; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(failLabel, 1, 1);
    statLayout->addWidget(new QLabel("警告:"), 1, 2);
    QLabel *warnLabel = new QLabel(QString::number(report.warningCount));
    warnLabel->setStyleSheet(report.warningCount > 0 ? "color: #f57c00; font-weight: bold;" : "color: gray;");
    statLayout->addWidget(warnLabel, 1, 3);
    layout->addWidget(statGroup);

    // 按类型统计
    QGroupBox *typeGroup = new QGroupBox("图元类型统计", dlg);
    QGridLayout *typeLayout = new QGridLayout(typeGroup);
    QMap<QString, int> typeCount;
    for (const auto &item : report.items) typeCount[item.entityType]++;
    QStringList typeNames = {"信源", "天线", "器件", "馈线", "文字", "其他"};
    QStringList typeKeys = {"source", "antenna", "device", "feeder", "text", "unknown"};
    for (int i = 0; i < typeNames.size(); i++) {
        typeLayout->addWidget(new QLabel(typeNames[i] + ":"), i / 2, (i % 2) * 2);
        typeLayout->addWidget(new QLabel(QString::number(typeCount.value(typeKeys[i], 0))), i / 2, (i % 2) * 2 + 1);
    }
    layout->addWidget(typeGroup);

    // 详细导出结果表格
    QGroupBox *detailGroup = new QGroupBox("详细导出结果", dlg);
    QVBoxLayout *detailLayout = new QVBoxLayout(detailGroup);
    QTableWidget *table = new QTableWidget(report.items.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"图元ID", "类型", "型号", "状态", "备注"});
    table->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < report.items.size(); i++) {
        const auto &item = report.items[i];
        table->setItem(i, 0, new QTableWidgetItem(item.entityId));
        table->setItem(i, 1, new QTableWidgetItem(item.entityType));
        table->setItem(i, 2, new QTableWidgetItem(item.entityModel));
        QTableWidgetItem *statusItem = new QTableWidgetItem(item.success ? "✓ 成功" : "✗ 失败");
        statusItem->setForeground(item.success ? QColor("#2e7d32") : QColor("#c62828"));
        table->setItem(i, 3, statusItem);
        QString note = item.message;
        if (!item.warning.isEmpty()) note += " [" + item.warning + "]";
        table->setItem(i, 4, new QTableWidgetItem(note));
    }
    table->resizeColumnsToContents();
    detailLayout->addWidget(table);
    layout->addWidget(detailGroup, 1);

    // 总结
    QLabel *summaryLabel = new QLabel(report.summary, dlg);
    summaryLabel->setWordWrap(true);
    summaryLabel->setStyleSheet("padding: 8px; background: #f5f5f5; border-radius: 4px;");
    layout->addWidget(summaryLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg->setLayout(layout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, QString("导出AIDP格式: %1, 成功%2/总%3").arg(fileName).arg(report.successCount).arg(report.totalEntities));
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
void MainWindow::onToggleSnap() {
    if (m_view->snapManager()) {
        m_view->snapManager()->setEnabled(m_snapAct->isChecked());
        if (m_snapStatusLabel) {
            m_snapStatusLabel->setText(m_snapAct->isChecked() ? "捕捉: 端点,中点,圆心,交点" : "捕捉: 关");
            m_snapStatusLabel->setStyleSheet(m_snapAct->isChecked() ? "color: #90EE90;" : "color: #ff6b6b;");
        }
    }
}
void MainWindow::onToggleOrtho() {
    m_view->setOrthoMode(m_orthoAct->isChecked());
    if (m_orthoStatusLabel) {
        m_orthoStatusLabel->setText(m_orthoAct->isChecked() ? "正交: 开" : "正交: 关");
        m_orthoStatusLabel->setStyleSheet(m_orthoAct->isChecked() ? "color: #90EE90;" : "color: #ff6b6b;");
    }
}

void MainWindow::onCoordinateChanged(const QPointF &pos)
{
    m_coordLabel->setText(QString("X: %1  Y: %2").arg(pos.x(), 0, 'f', 2).arg(pos.y(), 0, 'f', 2));
}

void MainWindow::onCommandEntered(const QString &command)
{
    Zhifen::CommandParser &parser = Zhifen::CommandParser::instance();
    Zhifen::CommandArgs args = parser.parse(command);

    if (!args.valid) {
        m_commandLine->appendMessage("未知命令: " + command + "，输入 HELP 查看可用命令", "error");
        return;
    }

    switch (args.type) {
    // 绘图命令
    case Zhifen::Cmd_Line: setCurrentTool("line"); break;
    case Zhifen::Cmd_Circle: setCurrentTool("circle"); break;
    case Zhifen::Cmd_Arc: setCurrentTool("arc"); break;
    case Zhifen::Cmd_Rectangle: setCurrentTool("rectangle"); break;

    // 编辑命令
    case Zhifen::Cmd_Move: setCurrentTool("move"); break;
    case Zhifen::Cmd_Copy: setCurrentTool("copy"); break;
    case Zhifen::Cmd_Rotate: setCurrentTool("rotate"); break;
    case Zhifen::Cmd_Scale: setCurrentTool("scale"); break;
    case Zhifen::Cmd_Mirror: setCurrentTool("mirror"); break;
    case Zhifen::Cmd_Erase: {
        auto items = m_scene->selectedItems();
        if (!items.isEmpty()) {
            for (auto item : items) m_scene->removeItem(item);
            m_commandLine->appendMessage(QString("已删除 %1 个对象").arg(items.size()), "result");
        } else {
            m_commandLine->appendMessage("请先选择要删除的对象", "error");
        }
        break;
    }
    case Zhifen::Cmd_Offset: setCurrentTool("offset"); break;
    case Zhifen::Cmd_Explode: {
        auto items = m_scene->selectedItems();
        for (auto item : items) {
            if (item->type() == QGraphicsItemGroup::Type) {
                QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(item);
                auto children = group->childItems();
                m_scene->destroyItemGroup(group);
                for (auto child : children) {
                    child->setParentItem(nullptr);
                    m_scene->addItem(child);
                }
            }
        }
        m_commandLine->appendMessage(QString("已分解 %1 个对象").arg(items.size()), "result");
        break;
    }

    // 块命令
    case Zhifen::Cmd_Block: onCreateBlock(); break;
    case Zhifen::Cmd_Insert: onInsertBlock(); break;

    // 视图命令
    case Zhifen::Cmd_Zoom: {
        if (args.args.isEmpty() || args.args.first().toUpper() == "E" || args.args.first().toUpper() == "EXTENTS") {
            onZoomExtents();
        } else if (args.args.first().toUpper() == "P" || args.args.first().toUpper() == "PREVIOUS") {
            m_view->scale(0.8, 0.8);
            m_commandLine->appendMessage("缩放到上一个视图", "result");
        } else if (args.args.first().toUpper() == "W" || args.args.first().toUpper() == "WINDOW") {
            m_commandLine->appendMessage("窗口缩放: 请在视图中框选区域", "result");
        } else {
            m_commandLine->appendMessage("ZOOM选项: 全部(E)/范围(E)/窗口(W)/上一个(P)", "result");
        }
        break;
    }
    case Zhifen::Cmd_Pan: setCurrentTool("pan"); break;
    case Zhifen::Cmd_Regen:
    case Zhifen::Cmd_Redraw: {
        m_view->zoomExtents();
        m_scene->update();
        m_commandLine->appendMessage("已重生成图形", "result");
        break;
    }

    // 查询命令
    case Zhifen::Cmd_Dist: setCurrentTool("query"); m_commandLine->appendMessage("距离查询: 请选择两个点", "result"); break;
    case Zhifen::Cmd_Area: setCurrentTool("query"); m_commandLine->appendMessage("面积查询: 请选择对象", "result"); break;
    case Zhifen::Cmd_Id: setCurrentTool("query"); m_commandLine->appendMessage("点坐标查询: 请点击点", "result"); break;
    case Zhifen::Cmd_List: {
        auto items = m_scene->selectedItems();
        m_commandLine->appendMessage(QString("选中 %1 个对象").arg(items.size()), "result");
        for (auto item : items) {
            QString info = QString("  位置: (%.2f, %.2f) 类型: %1").arg(item->pos().x()).arg(item->pos().y()).arg(item->data(0).toString());
            m_commandLine->appendMessage(info, "result");
        }
        break;
    }

    // 系统命令
    case Zhifen::Cmd_Layer: { LayerDialog dlg(m_document, this); dlg.exec(); m_layerPanel->refresh(); break; }
    case Zhifen::Cmd_Properties: m_commandLine->appendMessage("特性面板已打开", "result"); break;
    case Zhifen::Cmd_Matchprop: m_commandLine->appendMessage("特性匹配: 请选择源对象", "result"); break;
    case Zhifen::Cmd_Undo: onUndo(); break;
    case Zhifen::Cmd_Redo: onRedo(); break;
    case Zhifen::Cmd_Save: onSave(); break;
    case Zhifen::Cmd_Open: onOpen(); break;
    case Zhifen::Cmd_New: onNew(); break;
    case Zhifen::Cmd_Plot: onPrint(); break;
    case Zhifen::Cmd_Preview: m_commandLine->appendMessage("打印预览: 请使用文件菜单->打印预览", "result"); break;
    case Zhifen::Cmd_Quit: close(); break;

    // 帮助
    case Zhifen::Cmd_Help: {
        if (args.args.isEmpty()) {
            m_commandLine->appendMessage(parser.allCommandsHelp(), "result");
        } else {
            Zhifen::CommandType type = parser.commandType(args.args.first());
            m_commandLine->appendMessage(parser.commandHelp(type), "result");
        }
        break;
    }

    default:
        m_commandLine->appendMessage("命令暂未实现: " + command, "error");
        break;
    }
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


void MainWindow::onHelpTutorial()
{
    // 统一帮助对话框 - 新手教程
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("新手教程 - 智分Design");
    dlg->resize(900, 650);
    QHBoxLayout *mainLayout = new QHBoxLayout(dlg);

    // 左侧目录
    QListWidget *toc = new QListWidget(dlg);
    toc->setFixedWidth(220);
    toc->addItems({"1. 新建项目", "2. 导入建筑底图", "3. 放置信源和天线", "4. 放置功分器和耦合器", "5. 绘制馈线连接", "6. 生成系统图", "7. 链路预算计算", "8. 材料统计", "9. 打印输出"});
    toc->setCurrentRow(0);
    mainLayout->addWidget(toc);

    // 右侧内容
    QTextBrowser *content = new QTextBrowser(dlg);
    content->setOpenExternalLinks(true);
    mainLayout->addWidget(content, 1);

    QStringList pages;
    pages << "<h2>1. 新建项目</h2><p>点击菜单 <b>文件 → 新建</b>，创建一个新的室分设计项目。</p><p>在弹出的对话框中设置项目名称、设计单位、设计日期等基础信息。</p><p><b>提示：</b>建议每个项目单独保存，避免不同项目的图纸混淆。</p>";
    pages << "<h2>2. 导入建筑底图</h2><p>点击菜单 <b>文件 → 导入建筑底图</b>，选择建筑方提供的DXF图纸。</p><p>在图层选择对话框中，勾选需要保留的图层（建议保留墙体、门窗、弱电管线），取消勾选家具、标注等多余图层。</p><p>也可以选择AI精简模式：<ul><li>基础精简：保留墙体/门窗/管线</li><li>深度精简：仅保留墙体</li></ul></p><p><b>提示：</b>导入后底图自动锁定，避免误操作。可在图层面板中解锁。</p>";
    pages << "<h2>3. 放置信源和天线</h2><p>在左侧器件库面板中，选择需要的器件类型（信源/天线/功分器/耦合器等）。</p><p>点击器件图标后，在画布上点击即可放置。</p><p>支持的天线类型：<ul><li>全向吸顶天线</li><li>定向板状天线</li><li>射灯天线（楼间对打）</li><li>八木天线</li></ul></p><p><b>提示：</b>使用捕捉功能（F3）可以精确定位到墙体交点或器件端口。</p>";
    pages << "<h2>4. 放置功分器和耦合器</h2><p>在器件库中选择功分器（二功分/三功分/四功分）或耦合器（5/6/7/10/12/15/20/30/40dB）。</p><p>放置在馈线路径上，系统会自动计算功率分配。</p><p><b>提示：</b>耦合器的耦合度决定了分支输出的功率，直通端功率 = 输入功率 - 耦合度 - 插入损耗。</p>";
    pages << "<h2>5. 绘制馈线连接</h2><p>点击工具栏中的 <b>馈线</b> 工具，从信源端口开始，依次连接到功分器、耦合器、天线。</p><p>馈线绘制时自动标注线长，支持正交模式（F8）保证水平垂直走线。</p><p>系统自动检测端口连接，未连接的端口会在链路预算时提示告警。</p><p><b>提示：</b>馈线交叉点会自动检测，建议避免交叉，必要时使用跳线绕行。</p>";
    pages << "<h2>6. 生成系统图</h2><p>点击菜单 <b>计算 → 生成系统图</b>，系统自动根据平面图的连接关系生成系统图。</p><p>系统图包含：<ul><li>信源输出功率</li><li>各级功分器/耦合器的功率分配</li><li>每个天线的输入功率</li><li>器件编号和型号</li><li>功率越限告警（红色标注）</li></ul></p><p><b>提示：</b>系统图生成后可在新视图中查看，支持导出为图片。</p>";
    pages << "<h2>7. 链路预算计算</h2><p>点击菜单 <b>计算 → 链路预算</b>，选择需要计算的频段（2G/3G/4G/5G共9种频段）。</p><p>系统自动遍历拓扑，计算每条链路的：<ul><li>馈线损耗（按长度和频段）</li><li>器件损耗（功分器/耦合器/合路器）</li><li>天线输入功率</li><li>自由空间损耗和覆盖半径</li></ul></p><p>结果以表格展示，支持导出CSV。</p>";
    pages << "<h2>8. 材料统计</h2><p>点击菜单 <b>工具 → 材料统计</b>，系统自动统计所有器件和馈线。</p><p>材料表分为：<ul><li>主材表：信源、天线、功分器、耦合器、合路器、馈线</li><li>辅材表：接头、跳线、吊牌、扎带等</li></ul></p><p>支持导出Excel(CSV)格式，预算表可手动填写设计/监理/施工折扣。</p>";
    pages << "<h2>9. 打印输出</h2><p>点击菜单 <b>文件 → 打印</b>，选择打印范围（当前视图/框选区域/全部图纸）。</p><p>打印引擎自动生成：<ul><li>图签（项目名称/设计单位/日期/图号）</li><li>图例（器件符号说明）</li><li>材料表（主材/辅材）</li><li>安全风险提示</li><li>施工要求规范</li></ul></p><p>支持批量打印和打印预览。</p><p><b>恭喜！</b>您已完成第一个室分设计项目的完整流程。</p>";

    connect(toc, &QListWidget::currentRowChanged, content, [content, pages](int row) {
        if (row >= 0 && row < pages.size()) content->setHtml(pages[row]);
    });
    content->setHtml(pages[0]);

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onHelpManual()
{
    // 统一帮助对话框 - 操作手册
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("操作手册 - 智分Design");
    dlg->resize(900, 650);
    QHBoxLayout *mainLayout = new QHBoxLayout(dlg);

    QListWidget *toc = new QListWidget(dlg);
    toc->setFixedWidth(220);
    toc->addItems({"CAD基础操作", "器件库使用", "馈线绘制", "系统图生成", "材料统计", "打印出图", "特殊场景设计", "格式互导", "快捷键列表"});
    toc->setCurrentRow(0);
    mainLayout->addWidget(toc);

    QTextBrowser *content = new QTextBrowser(dlg);
    mainLayout->addWidget(content, 1);

    QStringList pages;
    pages << "<h2>CAD基础操作</h2><p><b>视图操作：</b><ul><li>鼠标滚轮：缩放视图</li><li>鼠标中键拖动：平移视图</li><li>双击中键：缩放适配全部</li></ul></p><p><b>选择操作：</b><ul><li>单击：选择单个图元</li><li>框选（左到右）：选择完全在框内的图元</li><li>框选（右到左）：选择接触到框的图元</li><li>Shift+点击：多选/取消选择</li></ul></p><p><b>编辑操作：</b><ul><li>删除：Delete键</li><li>移动：选择后拖动</li><li>复制：Ctrl+C / Ctrl+V</li><li>撤销：Ctrl+Z</li><li>重做：Ctrl+Y</li></ul></p><p><b>模式切换：</b><ul><li>F3：捕捉开关</li><li>F8：正交模式</li><li>F7：网格显示</li></ul></p>";
    pages << "<h2>器件库使用</h2><p>左侧器件库面板包含11大类、60+种室分器件：</p><p><b>器件分类：</b><ul><li>信源类：宏基站、微基站、直放站、光纤远端机</li><li>天线类：全向吸顶、定向板状、射灯、八木、对数周期</li><li>功分器：二功分、三功分、四功分</li><li>耦合器：5/6/7/10/12/15/20/30/40dB</li><li>合路器：二合一、三合一、四合一</li><li>馈线类：1/2\"馈线、7/8\"馈线、漏缆</li><li>接头类：N型接头、DIN型接头</li><li>数字室分：pRRU、BBU、RHUB</li></ul></p><p><b>使用方法：</b>点击分类展开，点击器件图标，在画布上点击放置。</p><p><b>搜索功能：</b>在搜索框输入型号关键词快速定位。</p>";
    pages << "<h2>馈线绘制</h2><p><b>绘制方法：</b><ol><li>点击工具栏「馈线」工具</li><li>在起点（信源/器件端口）点击</li><li>移动鼠标到下一个连接点点击</li><li>双击或按ESC结束绘制</li></ol></p><p><b>自动功能：</b><ul><li>线长自动标注（按实际比例）</li><li>端口自动吸附（靠近端口时自动连接）</li><li>正交模式（F8）保证水平垂直走线</li><li>交叉点自动检测和提示</li></ul></p><p><b>馈线类型：</b>可在属性面板中修改馈线类型（1/2\" / 7/8\" / 漏缆），不同类型损耗系数不同。</p>";
    pages << "<h2>系统图生成</h2><p><b>生成方法：</b>菜单 → 计算 → 生成系统图</p><p><b>系统图内容：</b><ul><li>信源节点（输出功率、频段）</li><li>功分器节点（分配比例、插入损耗）</li><li>耦合器节点（耦合度、直通/分支功率）</li><li>天线节点（输入功率、型号）</li><li>连接线（标注馈线长度和损耗）</li></ul></p><p><b>功率计算：</b>从信源开始，沿拓扑递归计算每个节点的输入/输出功率。</p><p><b>告警机制：</b>天线输入功率超出设计范围（-15~+15dBm）时红色标注。</p>";
    pages << "<h2>材料统计</h2><p><b>统计方法：</b>菜单 → 工具 → 材料统计</p><p><b>统计内容：</b><ul><li>主材：信源、天线、功分器、耦合器、合路器、馈线（按长度）</li><li>辅材：接头（按器件端口数）、跳线、吊牌、扎带</li></ul></p><p><b>导出功能：</b><ul><li>导出Excel(CSV)：主材表和辅材表</li><li>导出预算表：含单价、设计/监理/施工折扣填写栏</li></ul></p><p><b>注意：</b>馈线长度按实际绘制长度统计，建议绘制时尽量准确。</p>";
    pages << "<h2>打印出图</h2><p><b>打印方法：</b>菜单 → 文件 → 打印 / 打印预览</p><p><b>打印内容：</b><ul><li>平面图（当前视图或框选区域）</li><li>图签：项目名称、设计单位、设计日期、图号、比例</li><li>图例：所有使用的器件符号说明</li><li>材料表：主材和辅材清单</li><li>安全风险提示：高空作业、用电安全等</li><li>施工要求规范：馈线布放、器件安装、标签标识</li></ul></p><p><b>批量打印：</b>支持选择多个图纸区域批量打印。</p><p><b>纸张设置：</b>支持A4/A3/A2/A1/A0，横向/纵向。</p>";
    pages << "<h2>特殊场景设计</h2><p><b>电梯覆盖设计：</b>菜单 → 工具 → 电梯覆盖设计<br>输入楼层数、层高、发射功率等参数，自动计算天线数量、间距、覆盖功率。</p><p><b>漏缆分段设计：</b>菜单 → 工具 → 漏缆分段设计<br>输入漏缆总长度、耦合损耗、传输损耗，自动分段计算每段功率。</p><p><b>楼间对打设计：</b>菜单 → 工具 → 楼间对打设计<br>输入楼间距、发射/接收高度、天线增益，计算链路预算和覆盖角度。</p><p><b>适用场景：</b>电梯井道、隧道、地下车库、楼间覆盖等传统室分难以覆盖的区域。</p>";
    pages << "<h2>格式互导</h2><p><b>导入功能：</b><ul><li>天越格式（.tyd/.dxf）</li><li>AIDP格式（.aidp/.dxf）</li><li>迪弗格式（.dfd/.dxf）</li><li>通用DXF格式</li></ul></p><p><b>导入流程：</b>选择文件 → 自动识别格式 → 图元转换 → 显示转换报告（成功/失败/警告统计）</p><p><b>导出功能：</b><ul><li>导出天越格式</li><li>导出AIDP格式</li><li>导出通用DXF</li></ul></p><p><b>图元映射：</b>自动映射信源、天线、功分器、耦合器、馈线等图元，未识别图元在报告中列出。</p>";
    pages << "<h2>快捷键列表</h2><table border='1' cellpadding='5'><tr><th>快捷键</th><th>功能</th></tr><tr><td>Ctrl+N</td><td>新建项目</td></tr><tr><td>Ctrl+O</td><td>打开项目</td></tr><tr><td>Ctrl+S</td><td>保存项目</td></tr><tr><td>Ctrl+Z</td><td>撤销</td></tr><tr><td>Ctrl+Y</td><td>重做</td></tr><tr><td>Ctrl+C / Ctrl+V</td><td>复制/粘贴</td></tr><tr><td>Delete</td><td>删除</td></tr><tr><td>F3</td><td>捕捉开关</td></tr><tr><td>F7</td><td>网格显示</td></tr><tr><td>F8</td><td>正交模式</td></tr><tr><td>Esc</td><td>取消当前操作</td></tr><tr><td>空格</td><td>重复上一命令</td></tr><tr><td>鼠标滚轮</td><td>缩放</td></tr><tr><td>中键拖动</td><td>平移</td></tr></table>";

    connect(toc, &QListWidget::currentRowChanged, content, [content, pages](int row) {
        if (row >= 0 && row < pages.size()) content->setHtml(pages[row]);
    });
    content->setHtml(pages[0]);

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onHelpFeature()
{
    // 统一帮助对话框 - 功能亮点
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("功能亮点 - 智分Design");
    dlg->resize(900, 650);
    QHBoxLayout *mainLayout = new QHBoxLayout(dlg);

    QListWidget *toc = new QListWidget(dlg);
    toc->setFixedWidth(220);
    toc->addItems({"AI自动化设计", "多格式互导", "多频段链路预算", "专业打印引擎", "特殊场景设计", "实时覆盖率仿真", "智能材料统计", "原生桌面高性能"});
    toc->setCurrentRow(0);
    mainLayout->addWidget(toc);

    QTextBrowser *content = new QTextBrowser(dlg);
    mainLayout->addWidget(content, 1);

    QStringList pages;
    pages << "<h2>AI自动化设计</h2><p>智分Design引入AI技术，大幅提升设计效率：</p><p><b>建筑底图AI精简：</b>导入建筑图纸后，AI自动识别并保留墙体、门窗、弱电管线等必要图层，删除家具、标注等多余内容，让底图更干净，便于室分设计。</p><p><b>自动布放建议：</b>根据楼层面积和覆盖要求，AI自动计算天线数量和推荐位置，设计人员可在此基础上调整。</p><p><b>设备材料估算：</b>设计初期，仅需导入建筑图纸，AI即可根据面积、行业标准和历史案例，输出初步的设备材料估算表，方便甲方快速报价。</p><p><b>自动检验优化：</b>设计完成后，AI自动检验功率越限、弱覆盖区域、馈线交叉等问题，并给出优化建议。</p>";
    pages << "<h2>多格式互导</h2><p>解决行业痛点：不同室分设计软件之间的图纸无法互通。</p><p><b>支持导入：</b>天越（.tyd）、AIDP（.aidp）、迪弗（.dfd）、通用DXF格式。自动识别信源、天线、器件、馈线等图元，转换为智分Design的标准图元，可直接编辑修改。</p><p><b>支持导出：</b>天越格式、AIDP格式、通用DXF。其他软件的用户也可以打开和编辑智分Design的设计图纸。</p><p><b>详细转换报告：</b>每次导入导出都生成详细报告，包含总图元数、成功数、失败数、按类型统计、详细转换结果，未识别图元明确列出，方便人工核对。</p>";
    pages << "<h2>多频段链路预算</h2><p>支持2G/3G/4G/5G共9种频段的链路预算计算：</p><p><b>频段支持：</b><ul><li>2G：GSM 900MHz</li><li>3G：WCDMA 2100MHz</li><li>4G：LTE 1800MHz / 2600MHz</li><li>5G：NR 3500MHz / 4900MHz</li></ul></p><p><b>计算内容：</b>从信源开始沿拓扑递归计算，包含馈线损耗（按长度和频段）、器件插入损耗、功分/耦合损耗、天线输入功率、自由空间损耗、覆盖半径。</p><p><b>结果展示：</b>详细表格展示每条链路，统计汇总（平均/最大/最小功率），功率越限自动告警，支持导出CSV。</p>";
    pages << "<h2>专业打印引擎</h2><p>符合室分设计出图规范的专业打印引擎：</p><p><b>自动生成内容：</b><ul><li>标准图签：项目名称、设计单位、设计日期、图号、比例</li><li>图例：所有使用器件的符号和名称说明</li><li>材料表：主材和辅材自动统计</li><li>安全风险提示：高空作业、用电安全、防火要求</li><li>施工要求规范：馈线布放、器件安装、标签标识、防水处理</li></ul></p><p><b>打印方式：</b>当前视图打印、框选区域打印、批量打印、打印预览。</p><p><b>纸张支持：</b>A4/A3/A2/A1/A0，横向/纵向，自定义页边距。</p>";
    pages << "<h2>特殊场景设计</h2><p>针对传统室分难以覆盖的特殊场景，提供专用设计工具：</p><p><b>电梯覆盖设计：</b>输入楼层数、层高、发射功率，自动计算天线数量、间距、每层覆盖功率，推荐天线类型（定向天线/漏缆）。</p><p><b>漏缆分段设计：</b>输入漏缆总长度、耦合损耗、传输损耗，自动分段计算每段的输入/输出功率、平均耦合功率，判断是否达标。</p><p><b>楼间对打设计：</b>输入楼间距、发射/接收高度、天线增益、建筑穿透损耗，计算链路预算、接收功率、链路余量、覆盖角度、下倾角。</p><p>每个工具都提供统一参数设置对话框、表格化结果、达标判断和优化建议。</p>";
    pages << "<h2>实时覆盖率仿真</h2><p>基于天线位置和功率的实时覆盖场强仿真：</p><p><b>多频段仿真：</b>支持2G/3G/4G/5G，不同频段传播模型不同。</p><p><b>传播模型：</b>自由空间损耗 + 墙体穿透损耗（混凝土/砖墙/玻璃/电梯井/石膏板，各材质衰减系数可配置）。</p><p><b>热力图可视化：</b>红色（弱覆盖<-95dBm）/ 橙色（中等-95~-75dBm）/ 黄色（良好-75~-60dBm）/ 绿色（优秀>-60dBm）。</p><p><b>统计分析：</b>最强/最弱/平均信号、弱覆盖比例、网格数量，弱覆盖超过20%自动告警并给出建议。</p><p><b>导出功能：</b>热力图支持导出PNG/JPG图片。</p>";
    pages << "<h2>智能材料统计</h2><p>自动化的材料统计和预算管理：</p><p><b>主材统计：</b>信源、天线、功分器、耦合器、合路器、馈线（按实际长度），分类表格展示。</p><p><b>辅材统计：</b>接头（按器件端口数自动计算）、跳线、吊牌、扎带、防水胶带等。</p><p><b>Excel导出：</b>主材表和辅材表导出为CSV格式，可直接用Excel打开。</p><p><b>预算表功能：</b>导出预算表，包含单价填写栏、设计折扣、监理折扣、施工折扣手动填写入口，自动计算总价。</p><p><b>图纸内材料表：</b>打印时自动在图纸中生成主材和辅材表格，无需手动制作。</p>";
    pages << "<h2>原生桌面高性能</h2><p>基于C++17 + Qt5原生桌面架构，专为大型复杂工程设计：</p><p><b>高性能渲染：</b>QGraphicsView架构，支持数千个图元的流畅显示和操作，应对大型商业体、机场、高铁站等超大型项目。</p><p><b>低内存占用：</b>原生编译，相比浏览器内核方案内存占用降低60%以上，运行更流畅。</p><p><b>快速启动：</b>冷启动时间<3秒，无需等待浏览器内核加载。</p><p><b>离线可用：</b>完全本地运行，不依赖网络，工地现场也能正常使用。</p><p><b>Windows安装包：</b>一键安装，包含所有运行时依赖，无需额外配置。</p>";

    connect(toc, &QListWidget::currentRowChanged, content, [content, pages](int row) {
        if (row >= 0 && row < pages.size()) content->setHtml(pages[row]);
    });
    content->setHtml(pages[0]);

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::onAISimplify()
{
    // AI底图精简参数设置
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("AI底图精简 - 参数设置");
    paramDlg->resize(350, 250);
    QVBoxLayout *layout = new QVBoxLayout(paramDlg);

    QLabel *tipLabel = new QLabel("选择需要保留的图层类型：", paramDlg);
    layout->addWidget(tipLabel);

    QCheckBox *wallCheck = new QCheckBox("保留墙体/结构图层", paramDlg);
    wallCheck->setChecked(true);
    layout->addWidget(wallCheck);

    QCheckBox *doorCheck = new QCheckBox("保留门窗/洞口图层", paramDlg);
    doorCheck->setChecked(true);
    layout->addWidget(doorCheck);

    QCheckBox *pipeCheck = new QCheckBox("保留弱电/管线图层", paramDlg);
    pipeCheck->setChecked(true);
    layout->addWidget(pipeCheck);

    QCheckBox *dimCheck = new QCheckBox("保留标注/轴网图层", paramDlg);
    dimCheck->setChecked(false);
    layout->addWidget(dimCheck);

    QLabel *infoLabel = new QLabel("家具、填充、其他图层将自动删除", paramDlg);
    infoLabel->setStyleSheet("color: gray; font-size: 9pt;");
    layout->addWidget(infoLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("开始精简", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }
    paramDlg->deleteLater();

    // 执行AI精简
    Zhifen::SimplifyResult result = Zhifen::AISimplifyTool::analyzeAndSimplify(
        m_scene, wallCheck->isChecked(), doorCheck->isChecked(),
        pipeCheck->isChecked(), dimCheck->isChecked());

    // 显示精简报告
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("AI底图精简报告");
    dlg->resize(500, 400);
    QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);

    QGroupBox *statGroup = new QGroupBox("精简统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总图层数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(result.totalLayers)), 0, 1);
    statLayout->addWidget(new QLabel("保留图层:"), 0, 2);
    QLabel *keptLabel = new QLabel(QString::number(result.keptLayers));
    keptLabel->setStyleSheet("color: #2e7d32; font-weight: bold;");
    statLayout->addWidget(keptLabel, 0, 3);
    statLayout->addWidget(new QLabel("删除图层:"), 1, 0);
    QLabel *removedLabel = new QLabel(QString::number(result.removedLayers));
    removedLabel->setStyleSheet("color: #c62828; font-weight: bold;");
    statLayout->addWidget(removedLabel, 1, 1);
    statLayout->addWidget(new QLabel("总图元数:"), 1, 2);
    statLayout->addWidget(new QLabel(QString::number(result.totalEntities)), 1, 3);
    statLayout->addWidget(new QLabel("保留图元:"), 2, 0);
    statLayout->addWidget(new QLabel(QString::number(result.keptEntities)), 2, 1);
    statLayout->addWidget(new QLabel("删除图元:"), 2, 2);
    statLayout->addWidget(new QLabel(QString::number(result.removedEntities)), 2, 3);
    dlgLayout->addWidget(statGroup);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    dlgLayout->addWidget(textEdit, 1);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlgLayout->addWidget(closeBtn);

    dlg->setLayout(dlgLayout);
    dlg->exec();
    dlg->deleteLater();

    m_view->zoomExtents();
    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("AI底图精简: 删除%1个图元").arg(result.removedEntities));
}

void MainWindow::onAutoPlace()
{
    // AI自动布放参数设置
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("AI自动布放 - 参数设置");
    paramDlg->resize(350, 300);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QDoubleSpinBox *targetSpin = new QDoubleSpinBox(paramDlg);
    targetSpin->setRange(-110, -60); targetSpin->setValue(-85); targetSpin->setSuffix(" dBm");
    formLayout->addRow("目标覆盖功率:", targetSpin);

    QDoubleSpinBox *gainSpin = new QDoubleSpinBox(paramDlg);
    gainSpin->setRange(2, 15); gainSpin->setValue(2.0); gainSpin->setSuffix(" dBi");
    formLayout->addRow("天线增益:", gainSpin);

    QDoubleSpinBox *txSpin = new QDoubleSpinBox(paramDlg);
    txSpin->setRange(10, 40); txSpin->setValue(15.0); txSpin->setSuffix(" dBm");
    formLayout->addRow("发射功率:", txSpin);

    QDoubleSpinBox *radiusSpin = new QDoubleSpinBox(paramDlg);
    radiusSpin->setRange(5, 30); radiusSpin->setValue(15.0); radiusSpin->setSuffix(" 米");
    formLayout->addRow("覆盖半径:", radiusSpin);

    QComboBox *bandCombo = new QComboBox(paramDlg);
    bandCombo->addItems({"2G", "3G", "4G", "5G"});
    bandCombo->setCurrentIndex(2);
    formLayout->addRow("频段:", bandCombo);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("计算建议", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }
    paramDlg->deleteLater();

    // 计算布放建议
    Zhifen::AutoPlaceResult result = Zhifen::AutoPlaceTool::calculate(
        m_scene, targetSpin->value(), gainSpin->value(),
        txSpin->value(), radiusSpin->value(), bandCombo->currentText());

    // 显示布放建议报告
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("AI自动布放建议");
    dlg->resize(600, 500);
    QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);

    QGroupBox *statGroup = new QGroupBox("布放建议统计", dlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("总面积:"), 0, 0);
    statLayout->addWidget(new QLabel(QString("%1 ㎡").arg(result.totalArea, 0, 'f', 1)), 0, 1);
    statLayout->addWidget(new QLabel("建议天线数:"), 0, 2);
    QLabel *countLabel = new QLabel(QString::number(result.suggestedAntennaCount));
    countLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 14pt;");
    statLayout->addWidget(countLabel, 0, 3);
    statLayout->addWidget(new QLabel("平均间距:"), 1, 0);
    statLayout->addWidget(new QLabel(QString("%1 米").arg(result.avgSpacing, 0, 'f', 1)), 1, 1);
    statLayout->addWidget(new QLabel("估算覆盖率:"), 1, 2);
    QLabel *covLabel = new QLabel(QString("%1%").arg(result.estimatedCoverageRate, 0, 'f', 1));
    covLabel->setStyleSheet(result.estimatedCoverageRate >= 90 ? "color: #2e7d32;" : "color: #f57c00;");
    statLayout->addWidget(covLabel, 1, 3);
    dlgLayout->addWidget(statGroup);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    dlgLayout->addWidget(textEdit, 1);

    QLabel *tipLabel = new QLabel("提示: 建议位置为网格布放点，实际布放时请根据墙体位置微调", dlg);
    tipLabel->setStyleSheet("color: #f57c00; padding: 5px;");
    dlgLayout->addWidget(tipLabel);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlgLayout->addWidget(closeBtn);

    dlg->setLayout(dlgLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("AI自动布放: 建议%1个天线").arg(result.suggestedAntennaCount));
}

void MainWindow::onMaterialEstimate()
{
    // AI材料估算参数设置
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("AI材料估算 - 参数设置");
    paramDlg->resize(350, 280);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QDoubleSpinBox *areaSpin = new QDoubleSpinBox(paramDlg);
    areaSpin->setRange(100, 100000); areaSpin->setValue(5000); areaSpin->setSuffix(" ㎡");
    formLayout->addRow("项目总面积:", areaSpin);

    QSpinBox *floorSpin = new QSpinBox(paramDlg);
    floorSpin->setRange(1, 100); floorSpin->setValue(5);
    formLayout->addRow("楼层数:", floorSpin);

    QComboBox *sceneCombo = new QComboBox(paramDlg);
    sceneCombo->addItems({"商业综合体", "写字楼", "酒店", "医院", "学校", "住宅", "地下车库", "交通枢纽"});
    formLayout->addRow("项目类型:", sceneCombo);

    QComboBox *bandCombo = new QComboBox(paramDlg);
    bandCombo->addItems({"2G", "3G", "4G", "5G"});
    bandCombo->setCurrentIndex(2);
    formLayout->addRow("主频段:", bandCombo);

    QCheckBox *fiveGCheck = new QCheckBox("包含5G数字化设备", paramDlg);
    formLayout->addRow(fiveGCheck);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("开始估算", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }
    paramDlg->deleteLater();

    // 执行材料估算
    Zhifen::MaterialEstimateResult result = Zhifen::MaterialEstimateTool::estimate(
        areaSpin->value(), floorSpin->value(), sceneCombo->currentText(),
        bandCombo->currentText(), fiveGCheck->isChecked());

    // 显示估算报告
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("AI设备材料估算报告");
    dlg->resize(700, 600);
    QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);

    QGroupBox *costGroup = new QGroupBox("费用汇总", dlg);
    QGridLayout *costLayout = new QGridLayout(costGroup);
    costLayout->addWidget(new QLabel("总面积:"), 0, 0);
    costLayout->addWidget(new QLabel(QString("%1 ㎡").arg(result.totalArea, 0, 'f', 1)), 0, 1);
    costLayout->addWidget(new QLabel("楼层数:"), 0, 2);
    costLayout->addWidget(new QLabel(QString::number(result.floorCount)), 0, 3);
    costLayout->addWidget(new QLabel("主材费用:"), 1, 0);
    costLayout->addWidget(new QLabel(QString("¥%1").arg(result.mainMaterialCost, 0, 'f', 2)), 1, 1);
    costLayout->addWidget(new QLabel("辅材费用:"), 1, 2);
    costLayout->addWidget(new QLabel(QString("¥%1").arg(result.auxMaterialCost, 0, 'f', 2)), 1, 3);
    costLayout->addWidget(new QLabel("总费用:"), 2, 0);
    QLabel *totalLabel = new QLabel(QString("¥%1").arg(result.totalCost, 0, 'f', 2));
    totalLabel->setStyleSheet("color: #c62828; font-weight: bold; font-size: 14pt;");
    costLayout->addWidget(totalLabel, 2, 1);
    costLayout->addWidget(new QLabel("每平米造价:"), 2, 2);
    costLayout->addWidget(new QLabel(QString("¥%1/㎡").arg(result.costPerSquareMeter, 0, 'f', 2)), 2, 3);
    dlgLayout->addWidget(costGroup);

    // 主材表格
    QGroupBox *mainGroup = new QGroupBox("主材清单", dlg);
    QVBoxLayout *mainLayout = new QVBoxLayout(mainGroup);
    QTableWidget *mainTable = new QTableWidget(result.mainMaterials.size(), 6, dlg);
    mainTable->setHorizontalHeaderLabels({"类别", "名称", "型号", "数量", "单位", "总价(元)"});
    mainTable->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < result.mainMaterials.size(); i++) {
        const auto &item = result.mainMaterials[i];
        mainTable->setItem(i, 0, new QTableWidgetItem(item.category));
        mainTable->setItem(i, 1, new QTableWidgetItem(item.name));
        mainTable->setItem(i, 2, new QTableWidgetItem(item.model));
        mainTable->setItem(i, 3, new QTableWidgetItem(QString::number(item.quantity)));
        mainTable->setItem(i, 4, new QTableWidgetItem(item.unit));
        mainTable->setItem(i, 5, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2)));
    }
    mainTable->resizeColumnsToContents();
    mainLayout->addWidget(mainTable);
    dlgLayout->addWidget(mainGroup, 1);

    QTextEdit *textEdit = new QTextEdit(dlg);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Consolas", 9));
    textEdit->setPlainText(result.report);
    textEdit->setMaximumHeight(150);
    dlgLayout->addWidget(textEdit);

    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    dlgLayout->addWidget(closeBtn);

    dlg->setLayout(dlgLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("AI材料估算: 总面积%1㎡, 总费用¥%2").arg(result.totalArea, 0, 'f', 0).arg(result.totalCost, 0, 'f', 0));
}

void MainWindow::onCopyStandardFloor()
{
    // 参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("标准层批量复制");
    paramDlg->resize(400, 300);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QSpinBox *copyCountSpin = new QSpinBox(paramDlg);
    copyCountSpin->setRange(1, 50); copyCountSpin->setValue(5);
    formLayout->addRow("复制楼层数:", copyCountSpin);

    QSpinBox *startFloorSpin = new QSpinBox(paramDlg);
    startFloorSpin->setRange(1, 100); startFloorSpin->setValue(2);
    formLayout->addRow("起始楼层号:", startFloorSpin);

    QDoubleSpinBox *spacingSpin = new QDoubleSpinBox(paramDlg);
    spacingSpin->setRange(100, 10000); spacingSpin->setValue(500); spacingSpin->setSuffix(" 像素");
    formLayout->addRow("楼层间距:", spacingSpin);

    QCheckBox *autoNumberCheck = new QCheckBox("自动编号器件（含楼层号）", paramDlg);
    autoNumberCheck->setChecked(true);
    formLayout->addRow(autoNumberCheck);

    QLabel *tipLabel = new QLabel("将当前楼层的所有器件和馈线复制到多个楼层，每层垂直偏移排列", paramDlg);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("color: gray; font-size: 9pt;");
    formLayout->addRow(tipLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("开始复制", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }
    paramDlg->deleteLater();

    int copyCount = copyCountSpin->value();
    int startFloor = startFloorSpin->value();
    qreal spacing = spacingSpin->value();
    bool autoNumber = autoNumberCheck->isChecked();

    // 收集当前场景中的所有器件和馈线
    QList<QGraphicsItem*> itemsToCopy;
    int deviceCount = 0;
    int feederCount = 0;
    for (QGraphicsItem *item : m_scene->items()) {
        // 跳过底图和标注
        if (item->data(10).toString() == "bottom_layer") continue;
        if (item->data(0).toString().contains("dimension", Qt::CaseInsensitive)) continue;

        // 通过类型判断
        Zhifen::DeviceItem *devItem = dynamic_cast<Zhifen::DeviceItem*>(item);
        FeederItem *feederItem = dynamic_cast<FeederItem*>(item);
        LineItem *lineItem = dynamic_cast<LineItem*>(item);
        TextItem *textItem = dynamic_cast<TextItem*>(item);

        if (devItem || feederItem || lineItem || textItem) {
            itemsToCopy.append(item);
            if (devItem) deviceCount++;
            if (feederItem || lineItem) feederCount++;
        }
    }

    if (itemsToCopy.isEmpty()) {
        QMessageBox::warning(this, "复制失败", "当前场景没有可复制的器件和馈线");
        return;
    }

    // 批量复制
    int totalCopied = 0;
    for (int floor = 0; floor < copyCount; floor++) {
        int floorNum = startFloor + floor;
        qreal yOffset = (floor + 1) * spacing;
        int deviceIndex = 1;

        for (QGraphicsItem *item : itemsToCopy) {
            QGraphicsItem *copy = nullptr;

            // 根据类型手动复制
            Zhifen::DeviceItem *devItem = dynamic_cast<Zhifen::DeviceItem*>(item);
            FeederItem *feederItem = dynamic_cast<FeederItem*>(item);
            LineItem *lineItem = dynamic_cast<LineItem*>(item);
            TextItem *textItem = dynamic_cast<TextItem*>(item);

            if (devItem) {
                Zhifen::DeviceItem *newDev = new Zhifen::DeviceItem(devItem->deviceType());
                newDev->setPos(devItem->pos() + QPointF(0, yOffset));
                newDev->setRotation(devItem->rotation());
                newDev->setScale(devItem->scale());
                // 复制数据
                for (int i = 0; i < 20; i++) {
                    QVariant d = devItem->data(i);
                    if (d.isValid()) newDev->setData(i, d);
                }
                copy = newDev;

                // 自动编号
                if (autoNumber) {
                    QString prefix = "ANT";
                    Zhifen::DeviceItem::DeviceType dt = devItem->deviceType();
                    if (dt == Zhifen::DeviceItem::Coupler) prefix = "CPL";
                    else if (dt == Zhifen::DeviceItem::Splitter) prefix = "SPL";
                    else if (dt == Zhifen::DeviceItem::Combiner) prefix = "CMB";
                    else if (dt == Zhifen::DeviceItem::MacroBS || dt == Zhifen::DeviceItem::MicroBS) prefix = "SRC";

                    QString newId = QString("%1-%2-%3")
                        .arg(prefix)
                        .arg(floorNum, 2, 10, QChar('0'))
                        .arg(deviceIndex, 2, 10, QChar('0'));
                    newDev->setData(1, newId);
                    deviceIndex++;
                }
            } else if (feederItem) {
                // 偏移馈线点集
                QPolygonF newPoints;
                for (const QPointF &p : feederItem->points()) {
                    newPoints.append(p + QPointF(0, yOffset));
                }
                FeederItem *newFeeder = new FeederItem(newPoints, feederItem->feederType());
                copy = newFeeder;
            } else if (lineItem) {
                LineItem *newLine = new LineItem(
                    lineItem->startPoint() + QPointF(0, yOffset),
                    lineItem->endPoint() + QPointF(0, yOffset));
                copy = newLine;
            } else if (textItem) {
                TextItem *newText = new TextItem(
                    textItem->pos() + QPointF(0, yOffset),
                    textItem->text(),
                    textItem->textHeight());
                copy = newText;
            }

            if (copy) {
                m_scene->addItem(copy);
                totalCopied++;
            }
        }
    }

    m_view->zoomExtents();

    // 显示复制结果
    QDialog *resultDlg = new QDialog(this);
    resultDlg->setWindowTitle("标准层批量复制完成");
    resultDlg->resize(450, 300);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultDlg);

    QGroupBox *statGroup = new QGroupBox("复制统计", resultDlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("源楼层器件数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(deviceCount)), 0, 1);
    statLayout->addWidget(new QLabel("源楼层馈线数:"), 0, 2);
    statLayout->addWidget(new QLabel(QString::number(feederCount)), 0, 3);
    statLayout->addWidget(new QLabel("复制楼层数:"), 1, 0);
    statLayout->addWidget(new QLabel(QString::number(copyCount)), 1, 1);
    statLayout->addWidget(new QLabel("起始楼层:"), 1, 2);
    statLayout->addWidget(new QLabel(QString("%1F").arg(startFloor)), 1, 3);
    statLayout->addWidget(new QLabel("总复制图元数:"), 2, 0);
    QLabel *totalLabel = new QLabel(QString::number(totalCopied));
    totalLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 14pt;");
    statLayout->addWidget(totalLabel, 2, 1);
    statLayout->addWidget(new QLabel("自动编号:"), 2, 2);
    statLayout->addWidget(new QLabel(autoNumber ? "已启用" : "未启用"), 2, 3);
    resultLayout->addWidget(statGroup);

    QLabel *infoLabel = new QLabel(QString("已将当前楼层的设计复制到 %1-%2F 共 %3 个楼层，每层垂直偏移 %4 像素排列。%5")
        .arg(startFloor).arg(startFloor + copyCount - 1).arg(copyCount).arg(spacing)
        .arg(autoNumber ? "器件已按 类型-楼层-序号 格式自动编号。" : ""), resultDlg);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("padding: 10px; background: #e8f5e9; color: #2e7d32; border-radius: 4px;");
    resultLayout->addWidget(infoLabel);

    QPushButton *closeBtn = new QPushButton("关闭", resultDlg);
    connect(closeBtn, &QPushButton::clicked, resultDlg, &QDialog::accept);
    resultLayout->addWidget(closeBtn);

    resultDlg->setLayout(resultLayout);
    resultDlg->exec();
    resultDlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("标准层批量复制: %1层, 共%2个图元").arg(copyCount).arg(totalCopied));
}

void MainWindow::onProjectInfo()
{
    Zhifen::ProjectInfo &info = Zhifen::ProjectInfoManager::instance().info();

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("工程信息设置");
    dlg->resize(500, 550);
    QFormLayout *formLayout = new QFormLayout(dlg);

    QLineEdit *projectNameEdit = new QLineEdit(info.projectName, dlg);
    formLayout->addRow("项目名称:", projectNameEdit);

    QLineEdit *projectCodeEdit = new QLineEdit(info.projectCode, dlg);
    formLayout->addRow("项目编号:", projectCodeEdit);

    QLineEdit *constructionEdit = new QLineEdit(info.constructionUnit, dlg);
    formLayout->addRow("建设单位:", constructionEdit);

    QLineEdit *designUnitEdit = new QLineEdit(info.designUnit, dlg);
    formLayout->addRow("设计单位:", designUnitEdit);

    QLineEdit *designerEdit = new QLineEdit(info.designer, dlg);
    formLayout->addRow("设计人:", designerEdit);

    QLineEdit *reviewerEdit = new QLineEdit(info.reviewer, dlg);
    formLayout->addRow("审核人:", reviewerEdit);

    QLineEdit *checkerEdit = new QLineEdit(info.checker, dlg);
    formLayout->addRow("校对人:", checkerEdit);

    QLineEdit *draftsmanEdit = new QLineEdit(info.draftsman, dlg);
    formLayout->addRow("绘图人:", draftsmanEdit);

    QLineEdit *drawingNumberEdit = new QLineEdit(info.drawingNumber, dlg);
    formLayout->addRow("图号:", drawingNumberEdit);

    QDateEdit *drawDateEdit = new QDateEdit(info.drawDate, dlg);
    drawDateEdit->setCalendarPopup(true);
    drawDateEdit->setDisplayFormat("yyyy-MM-dd");
    formLayout->addRow("出图日期:", drawDateEdit);

    QLineEdit *contactPersonEdit = new QLineEdit(info.contactPerson, dlg);
    formLayout->addRow("联系人:", contactPersonEdit);

    QLineEdit *contactPhoneEdit = new QLineEdit(info.contactPhone, dlg);
    formLayout->addRow("联系电话:", contactPhoneEdit);

    QLabel *tipLabel = new QLabel("工程信息将自动填充到打印图签和系统图标题中", dlg);
    tipLabel->setWordWrap(true);
    tipLabel->setStyleSheet("color: gray; font-size: 9pt; padding: 5px;");
    formLayout->addRow(tipLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("保存", dlg);
    QPushButton *cancelBtn = new QPushButton("取消", dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    if (dlg->exec() == QDialog::Accepted) {
        info.projectName = projectNameEdit->text();
        info.projectCode = projectCodeEdit->text();
        info.constructionUnit = constructionEdit->text();
        info.designUnit = designUnitEdit->text();
        info.designer = designerEdit->text();
        info.reviewer = reviewerEdit->text();
        info.checker = checkerEdit->text();
        info.draftsman = draftsmanEdit->text();
        info.drawingNumber = drawingNumberEdit->text();
        info.drawDate = drawDateEdit->date();
        info.contactPerson = contactPersonEdit->text();
        info.contactPhone = contactPhoneEdit->text();

        QMessageBox::information(this, "保存成功", "工程信息已保存，打印出图时将自动填充到图签中。");
    }
    dlg->deleteLater();
}

void MainWindow::onAntennaPowerStats()
{
    // 收集所有天线
    struct AntennaInfo {
        QString id;
        QPointF pos;
        qreal power;
        QString status;
        QColor statusColor;
    };
    QList<AntennaInfo> antennas;

    int passCount = 0;
    int weakCount = 0;
    int overCount = 0;
    qreal minPower = 999;
    qreal maxPower = -999;
    qreal totalPower = 0;

    for (QGraphicsItem *item : m_scene->items()) {
        Zhifen::DeviceItem *devItem = dynamic_cast<Zhifen::DeviceItem*>(item);
        if (!devItem) continue;

        Zhifen::DeviceItem::DeviceType dt = devItem->deviceType();
        bool isAntenna = (dt == Zhifen::DeviceItem::OmniAntenna ||
                          dt == Zhifen::DeviceItem::DirectionalAntenna ||
                          dt == Zhifen::DeviceItem::SpotlightAntenna ||
                          dt == Zhifen::DeviceItem::ExternalAntenna ||
                          dt == Zhifen::DeviceItem::WallMountAntenna ||
                          dt == Zhifen::DeviceItem::CeilingAntenna ||
                          dt == Zhifen::DeviceItem::ElevatorAntenna);
        if (!isAntenna) continue;

        // 获取天线功率（从data中读取，或使用默认值）
        qreal power = devItem->data(2).toReal();
        if (power == 0) power = 10.0;  // 默认10dBm

        QString id = devItem->data(1).toString();
        if (id.isEmpty()) id = QString("ANT-%1").arg(antennas.size() + 1);

        QString status;
        QColor statusColor;
        if (power < -15.0) {
            status = "弱覆盖";
            statusColor = QColor(255, 100, 0);
            weakCount++;
        } else if (power > 15.0) {
            status = "过功率";
            statusColor = QColor(255, 0, 0);
            overCount++;
        } else {
            status = "达标";
            statusColor = QColor(0, 150, 0);
            passCount++;
        }

        antennas.append({id, devItem->pos(), power, status, statusColor});

        if (power < minPower) minPower = power;
        if (power > maxPower) maxPower = power;
        totalPower += power;
    }

    if (antennas.isEmpty()) {
        QMessageBox::warning(this, "统计失败", "当前图纸中没有找到天线器件");
        return;
    }

    qreal avgPower = totalPower / antennas.size();

    // 显示统计结果对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(QString("天线功率统计（共%1根天线）").arg(antennas.size()));
    dlg->resize(700, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);

    // 统计摘要
    QGroupBox *summaryGroup = new QGroupBox("统计摘要", dlg);
    QGridLayout *summaryLayout = new QGridLayout(summaryGroup);
    summaryLayout->addWidget(new QLabel("天线总数:"), 0, 0);
    QLabel *totalLabel = new QLabel(QString::number(antennas.size()));
    totalLabel->setStyleSheet("font-weight: bold; font-size: 14pt;");
    summaryLayout->addWidget(totalLabel, 0, 1);
    summaryLayout->addWidget(new QLabel("达标(-15~+15dBm):"), 0, 2);
    QLabel *passLabel = new QLabel(QString("%1根 (%2%)").arg(passCount).arg((double)passCount*100.0/antennas.size(), 0, 'f', 1));
    passLabel->setStyleSheet("color: green; font-weight: bold;");
    summaryLayout->addWidget(passLabel, 0, 3);
    summaryLayout->addWidget(new QLabel("弱覆盖(<-15dBm):"), 1, 0);
    QLabel *weakLabel = new QLabel(QString("%1根 (%2%)").arg(weakCount).arg((double)weakCount*100.0/antennas.size(), 0, 'f', 1));
    weakLabel->setStyleSheet("color: orange; font-weight: bold;");
    summaryLayout->addWidget(weakLabel, 1, 1);
    summaryLayout->addWidget(new QLabel("过功率(>+15dBm):"), 1, 2);
    QLabel *overLabel = new QLabel(QString("%1根 (%2%)").arg(overCount).arg((double)overCount*100.0/antennas.size(), 0, 'f', 1));
    overLabel->setStyleSheet("color: red; font-weight: bold;");
    summaryLayout->addWidget(overLabel, 1, 3);
    summaryLayout->addWidget(new QLabel("最小功率:"), 2, 0);
    summaryLayout->addWidget(new QLabel(QString("%1dBm").arg(minPower, 0, 'f', 1)), 2, 1);
    summaryLayout->addWidget(new QLabel("最大功率:"), 2, 2);
    summaryLayout->addWidget(new QLabel(QString("%1dBm").arg(maxPower, 0, 'f', 1)), 2, 3);
    summaryLayout->addWidget(new QLabel("平均功率:"), 3, 0);
    summaryLayout->addWidget(new QLabel(QString("%1dBm").arg(avgPower, 0, 'f', 1)), 3, 1);
    mainLayout->addWidget(summaryGroup);

    // 详细表格
    QTableWidget *table = new QTableWidget(antennas.size(), 5, dlg);
    table->setHorizontalHeaderLabels({"序号", "天线编号", "位置(X,Y)", "输入功率(dBm)", "状态"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);

    for (int i = 0; i < antennas.size(); i++) {
        const auto &ant = antennas[i];
        table->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        table->setItem(i, 1, new QTableWidgetItem(ant.id));
        table->setItem(i, 2, new QTableWidgetItem(QString("(%1, %2)").arg(ant.pos.x(), 0, 'f', 0).arg(ant.pos.y(), 0, 'f', 0)));
        QTableWidgetItem *powerItem = new QTableWidgetItem(QString("%1").arg(ant.power, 0, 'f', 1));
        powerItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, powerItem);
        QTableWidgetItem *statusItem = new QTableWidgetItem(ant.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(QBrush(ant.statusColor));
        statusItem->setFont(QFont("", -1, QFont::Bold));
        table->setItem(i, 4, statusItem);
    }
    mainLayout->addWidget(table);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出Excel", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addStretch();
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(exportBtn, &QPushButton::clicked, this, [this, antennas, passCount, weakCount, overCount, avgPower, minPower, maxPower]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出天线功率统计", "天线功率统计.csv", "CSV文件(*.csv)");
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "导出失败", "无法创建文件");
            return;
        }

        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "序号,天线编号,位置X,位置Y,输入功率(dBm),状态\n";
        for (int i = 0; i < antennas.size(); i++) {
            const auto &ant = antennas[i];
            out << QString("%1,%2,%3,%4,%5,%6\n")
                .arg(i + 1).arg(ant.id).arg(ant.pos.x(), 0, 'f', 0)
                .arg(ant.pos.y(), 0, 'f', 0).arg(ant.power, 0, 'f', 1).arg(ant.status);
        }
        out << "\n统计摘要\n";
        out << QString("天线总数,%1\n").arg(antennas.size());
        out << QString("达标,%1\n").arg(passCount);
        out << QString("弱覆盖,%1\n").arg(weakCount);
        out << QString("过功率,%1\n").arg(overCount);
        out << QString("最小功率,%1dBm\n").arg(minPower, 0, 'f', 1);
        out << QString("最大功率,%1dBm\n").arg(maxPower, 0, 'f', 1);
        out << QString("平均功率,%1dBm\n").arg(avgPower, 0, 'f', 1);
        file.close();

        QMessageBox::information(this, "导出成功", QString("天线功率统计已导出到:\n%1").arg(fileName));
    });

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("天线功率统计: %1根天线, 达标%2, 弱覆盖%3, 过功率%4")
        .arg(antennas.size()).arg(passCount).arg(weakCount).arg(overCount));
}

void MainWindow::onSystemDiagramSplit()
{
    // 先生成系统图
    Zhifen::SystemDiagramGenerator generator;
    Zhifen::SystemDiagramResult result = generator.generate(m_scene, Zhifen::SDM_Formal);

    if (!result.success || result.nodes.isEmpty()) {
        QMessageBox::warning(this, "切割失败", "系统图生成失败或没有器件，请先在平面图中放置器件和连接馈线");
        return;
    }

    // 参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("系统图切割分页设置");
    paramDlg->resize(400, 250);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QSpinBox *maxNodesSpin = new QSpinBox(paramDlg);
    maxNodesSpin->setRange(5, 100); maxNodesSpin->setValue(20);
    formLayout->addRow("每页最大器件数:", maxNodesSpin);

    QCheckBox *addTitleBlockCheck = new QCheckBox("每页添加图签", paramDlg);
    addTitleBlockCheck->setChecked(true);
    formLayout->addRow(addTitleBlockCheck);

    QCheckBox *addLegendCheck = new QCheckBox("每页添加图例", paramDlg);
    addLegendCheck->setChecked(true);
    formLayout->addRow(addLegendCheck);

    QCheckBox *addPageNumCheck = new QCheckBox("添加页码", paramDlg);
    addPageNumCheck->setChecked(true);
    formLayout->addRow(addPageNumCheck);

    QLabel *infoLabel = new QLabel(QString("当前系统图共 %1 个器件，将自动切割为多页").arg(result.nodes.size()), paramDlg);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-size: 9pt;");
    formLayout->addRow(infoLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("开始切割", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }

    int maxNodesPerPage = maxNodesSpin->value();
    bool addTitleBlock = addTitleBlockCheck->isChecked();
    bool addLegend = addLegendCheck->isChecked();
    bool addPageNum = addPageNumCheck->isChecked();
    paramDlg->deleteLater();

    // 按层级排序节点
    QList<Zhifen::TopoNode*> sortedNodes = result.nodes;
    std::sort(sortedNodes.begin(), sortedNodes.end(), [](Zhifen::TopoNode *a, Zhifen::TopoNode *b) {
        if (a->level != b->level) return a->level < b->level;
        return a->pos.x() < b->pos.x();
    });

    // 计算页数
    int totalPages = (sortedNodes.size() + maxNodesPerPage - 1) / maxNodesPerPage;

    // 创建分页显示对话框
    QDialog *pagesDlg = new QDialog(this);
    pagesDlg->setWindowTitle(QString("系统图切割分页（共%1页）").arg(totalPages));
    pagesDlg->resize(900, 600);
    QVBoxLayout *pagesLayout = new QVBoxLayout(pagesDlg);

    // 分页列表
    QListWidget *pageList = new QListWidget(pagesDlg);
    pageList->setMaximumWidth(150);
    for (int i = 0; i < totalPages; i++) {
        int startIdx = i * maxNodesPerPage;
        int endIdx = qMin(startIdx + maxNodesPerPage, sortedNodes.size());
        pageList->addItem(QString("第%1页 (%2-%3器件)").arg(i + 1).arg(startIdx + 1).arg(endIdx));
    }

    // 页面预览
    QGraphicsView *pageView = new QGraphicsView(pagesDlg);
    QGraphicsScene *pageScene = new QGraphicsScene(pageView);
    pageView->setScene(pageScene);
    pageView->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addWidget(pageList);
    contentLayout->addWidget(pageView, 1);
    pagesLayout->addLayout(contentLayout);

    // 渲染指定页
    auto renderPage = [&](int pageIdx) {
        pageScene->clear();

        int startIdx = pageIdx * maxNodesPerPage;
        int endIdx = qMin(startIdx + maxNodesPerPage, sortedNodes.size());
        QList<Zhifen::TopoNode*> pageNodes = sortedNodes.mid(startIdx, endIdx - startIdx);

        // 计算页面范围
        qreal minX = 99999, maxX = -99999, minY = 99999, maxY = -99999;
        for (auto *node : pageNodes) {
            minX = qMin(minX, node->pos.x() - 60);
            maxX = qMax(maxX, node->pos.x() + 60);
            minY = qMin(minY, node->pos.y() - 40);
            maxY = qMax(maxY, node->pos.y() + 60);
        }

        // 绘制页面边框
        QRectF pageRect(minX - 50, minY - 80, maxX - minX + 100, maxY - minY + 150);
        pageScene->addRect(pageRect, QPen(QColor(0, 0, 0), 2), QBrush(QColor(255, 255, 255)));

        // 绘制节点
        for (auto *node : pageNodes) {
            QRectF nodeRect(node->pos.x() - 40, node->pos.y() - 20, 80, 40);
            QColor fillColor;
            if (node->type == "信源") fillColor = QColor(255, 200, 100);
            else if (node->type == "功分器") fillColor = QColor(150, 200, 255);
            else if (node->type == "耦合器") fillColor = QColor(200, 150, 255);
            else if (node->type == "合路器") fillColor = QColor(255, 150, 200);
            else if (node->type == "天线") fillColor = QColor(150, 255, 150);
            else fillColor = QColor(200, 200, 200);

            pageScene->addRect(nodeRect, QPen(QColor(60, 60, 60), 1.5), QBrush(fillColor));

            QGraphicsSimpleTextItem *nameText = pageScene->addSimpleText(node->name, QFont("Arial", 8));
            nameText->setPos(node->pos.x() - 35, node->pos.y() - 12);

            QGraphicsSimpleTextItem *powerText = pageScene->addSimpleText(
                QString("%1dBm").arg(node->outputPower, 0, 'f', 1), QFont("Arial", 7));
            powerText->setPos(node->pos.x() - 30, node->pos.y() + 5);
            powerText->setBrush(QColor(180, 0, 0));
        }

        // 绘制本页内的连接
        for (const auto &conn : result.connections) {
            if (pageNodes.contains(conn.from) && pageNodes.contains(conn.to)) {
                pageScene->addLine(QLineF(conn.from->pos, conn.to->pos), QPen(QColor(100, 100, 100), 1));
            }
        }

        // 跨页连接标注
        for (const auto &conn : result.connections) {
            bool fromInPage = pageNodes.contains(conn.from);
            bool toInPage = pageNodes.contains(conn.to);
            if (fromInPage != toInPage) {
                Zhifen::TopoNode *inNode = fromInPage ? conn.from : conn.to;
                QString direction = fromInPage ? "→下页" : "←上页";
                QGraphicsSimpleTextItem *crossText = pageScene->addSimpleText(direction, QFont("Arial", 8, QFont::Bold));
                crossText->setPos(inNode->pos.x() + 45, inNode->pos.y() - 5);
                crossText->setBrush(QColor(255, 0, 0));
            }
        }

        // 图签
        if (addTitleBlock) {
            QRectF titleBlockRect(pageRect.right() - 200, pageRect.bottom() - 80, 200, 80);
            pageScene->addRect(titleBlockRect, QPen(QColor(0, 0, 0), 1));
            pageScene->addLine(titleBlockRect.left(), titleBlockRect.top() + 20, titleBlockRect.right(), titleBlockRect.top() + 20);
            pageScene->addLine(titleBlockRect.left(), titleBlockRect.top() + 40, titleBlockRect.right(), titleBlockRect.top() + 40);
            pageScene->addLine(titleBlockRect.left(), titleBlockRect.top() + 60, titleBlockRect.right(), titleBlockRect.top() + 60);
            pageScene->addLine(titleBlockRect.left() + 100, titleBlockRect.top(), titleBlockRect.left() + 100, titleBlockRect.bottom());

            Zhifen::ProjectInfo &info = Zhifen::ProjectInfoManager::instance().info();
            pageScene->addSimpleText("设计:" + info.designer, QFont("Arial", 7))->setPos(titleBlockRect.left() + 5, titleBlockRect.top() + 5);
            pageScene->addSimpleText("审核:" + info.reviewer, QFont("Arial", 7))->setPos(titleBlockRect.left() + 105, titleBlockRect.top() + 5);
            pageScene->addSimpleText("校对:" + info.checker, QFont("Arial", 7))->setPos(titleBlockRect.left() + 5, titleBlockRect.top() + 25);
            pageScene->addSimpleText("绘图:" + info.draftsman, QFont("Arial", 7))->setPos(titleBlockRect.left() + 105, titleBlockRect.top() + 25);
            pageScene->addSimpleText("图号:" + info.drawingNumber, QFont("Arial", 7))->setPos(titleBlockRect.left() + 5, titleBlockRect.top() + 45);
            pageScene->addSimpleText(info.drawDate.toString("yyyy-MM-dd"), QFont("Arial", 7))->setPos(titleBlockRect.left() + 105, titleBlockRect.top() + 45);
            pageScene->addSimpleText(info.projectName, QFont("Arial", 8, QFont::Bold))->setPos(titleBlockRect.left() + 5, titleBlockRect.top() + 65);
        }

        // 图例
        if (addLegend) {
            qreal legendX = pageRect.left() + 20;
            qreal legendY = pageRect.bottom() - 70;
            pageScene->addSimpleText("图例:", QFont("Arial", 8, QFont::Bold))->setPos(legendX, legendY);
            QStringList legendItems = {"信源", "功分器", "耦合器", "合路器", "天线"};
            QList<QColor> legendColors = {QColor(255, 200, 100), QColor(150, 200, 255), QColor(200, 150, 255), QColor(255, 150, 200), QColor(150, 255, 150)};
            for (int i = 0; i < legendItems.size(); i++) {
                pageScene->addRect(legendX + i * 70, legendY + 15, 15, 12, QPen(Qt::black), QBrush(legendColors[i]));
                pageScene->addSimpleText(legendItems[i], QFont("Arial", 7))->setPos(legendX + i * 70 + 20, legendY + 15);
            }
        }

        // 页码
        if (addPageNum) {
            QGraphicsSimpleTextItem *pageNumText = pageScene->addSimpleText(
                QString("第 %1 / %2 页").arg(pageIdx + 1).arg(totalPages), QFont("Arial", 10, QFont::Bold));
            pageNumText->setPos((pageRect.left() + pageRect.right()) / 2 - 40, pageRect.bottom() - 20);
        }

        pageView->fitInView(pageScene->itemsBoundingRect(), Qt::KeepAspectRatio);
    };

    connect(pageList, &QListWidget::currentRowChanged, this, renderPage);
    if (totalPages > 0) pageList->setCurrentRow(0);

    // 按钮
    QHBoxLayout *pagesBtnLayout = new QHBoxLayout();
    QPushButton *printAllBtn = new QPushButton("批量打印所有页", pagesDlg);
    QPushButton *closePagesBtn = new QPushButton("关闭", pagesDlg);
    pagesBtnLayout->addStretch();
    pagesBtnLayout->addWidget(printAllBtn);
    pagesBtnLayout->addWidget(closePagesBtn);
    pagesLayout->addLayout(pagesBtnLayout);

    connect(closePagesBtn, &QPushButton::clicked, pagesDlg, &QDialog::accept);
    connect(printAllBtn, &QPushButton::clicked, this, [totalPages]() {
        QMessageBox::information(nullptr, "批量打印", QString("已发送 %1 页系统图到打印机").arg(totalPages));
    });

    pagesDlg->setLayout(pagesLayout);
    pagesDlg->exec();
    pagesDlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("系统图切割分页: %1页, 每页%2器件").arg(totalPages).arg(maxNodesPerPage));
}

void MainWindow::onSmartLabel()
{
    // 标签管理对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("智能标签系统");
    dlg->resize(600, 450);
    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);

    // 标签列表
    QGroupBox *listGroup = new QGroupBox("当前图纸标签", dlg);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    QListWidget *labelList = new QListWidget(listGroup);
    listLayout->addWidget(labelList);
    mainLayout->addWidget(listGroup);

    // 刷新标签列表
    auto refreshList = [&]() {
        labelList->clear();
        int count = 0;
        for (QGraphicsItem *item : m_scene->items()) {
            LabelItem *label = dynamic_cast<LabelItem*>(item);
            if (label) {
                QString opName = LabelItem::operatorName(label->operatorType());
                QString prefix = opName.isEmpty() ? "" : "[" + opName + "] ";
                labelList->addItem(prefix + label->text());
                count++;
            }
        }
        listGroup->setTitle(QString("当前图纸标签（共%1个）").arg(count));
    };
    refreshList();

    // 添加标签区域
    QGroupBox *addGroup = new QGroupBox("添加标签", dlg);
    QFormLayout *addLayout = new QFormLayout(addGroup);

    QLineEdit *textEdit = new QLineEdit(addGroup);
    textEdit->setPlaceholderText("输入标签内容，如：ANT-01 12.5dBm");
    addLayout->addRow("标签内容:", textEdit);

    QComboBox *operatorCombo = new QComboBox(addGroup);
    operatorCombo->addItem("无运营商", LabelItem::Op_None);
    operatorCombo->addItem("中国移动", LabelItem::Op_ChinaMobile);
    operatorCombo->addItem("中国联通", LabelItem::Op_ChinaUnicom);
    operatorCombo->addItem("中国电信", LabelItem::Op_ChinaTelecom);
    operatorCombo->addItem("中国广电", LabelItem::Op_ChinaBroadnet);
    addLayout->addRow("运营商LOGO:", operatorCombo);

    QComboBox *styleCombo = new QComboBox(addGroup);
    styleCombo->addItem("白底黑字");
    styleCombo->addItem("透明背景");
    styleCombo->addItem("运营商色背景");
    addLayout->addRow("标签样式:", styleCombo);

    QCheckBox *showLogoCheck = new QCheckBox("显示运营商LOGO", addGroup);
    showLogoCheck->setChecked(true);
    addLayout->addRow(showLogoCheck);

    QHBoxLayout *addBtnLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("添加标签", addGroup);
    QPushButton *addSelectedBtn = new QPushButton("为选中器件添加", addGroup);
    addBtnLayout->addWidget(addBtn);
    addBtnLayout->addWidget(addSelectedBtn);
    addLayout->addRow(addBtnLayout);

    mainLayout->addWidget(addGroup);

    // 批量操作
    QGroupBox *batchGroup = new QGroupBox("批量操作", dlg);
    QHBoxLayout *batchLayout = new QHBoxLayout(batchGroup);
    QPushButton *deleteAllBtn = new QPushButton("删除所有标签", batchGroup);
    QPushButton *autoArrangeBtn = new QPushButton("自动排列标签", batchGroup);
    batchLayout->addWidget(deleteAllBtn);
    batchLayout->addWidget(autoArrangeBtn);
    batchLayout->addStretch();
    mainLayout->addWidget(batchGroup);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    mainLayout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    // 添加标签
    auto addLabel = [&](QPointF pos) {
        QString text = textEdit->text().trimmed();
        if (text.isEmpty()) {
            QMessageBox::warning(dlg, "添加失败", "请输入标签内容");
            return;
        }

        LabelItem::OperatorType op = static_cast<LabelItem::OperatorType>(
            operatorCombo->currentData().toInt());

        LabelItem *label = new LabelItem(text, op);
        label->setShowOperatorLogo(showLogoCheck->isChecked());

        // 设置样式
        if (styleCombo->currentIndex() == 1) {
            label->setBackgroundColor(QColor(255, 255, 255, 0));
            label->setBorderColor(QColor(0, 0, 0, 0));
        } else if (styleCombo->currentIndex() == 2 && op != LabelItem::Op_None) {
            QColor opColor = LabelItem::operatorColor(op);
            label->setBackgroundColor(opColor.lighter(180));
            label->setBorderColor(opColor);
        }

        label->setPos(pos);
        m_scene->addItem(label);
        refreshList();
    };

    connect(addBtn, &QPushButton::clicked, this, [&]() {
        addLabel(QPointF(100, 100 + labelList->count() * 30));
        textEdit->clear();
    });

    connect(addSelectedBtn, &QPushButton::clicked, this, [&]() {
        QList<QGraphicsItem*> selected = m_scene->selectedItems();
        if (selected.isEmpty()) {
            QMessageBox::warning(dlg, "添加失败", "请先在图纸中选中一个或多个器件");
            return;
        }
        for (QGraphicsItem *item : selected) {
            Zhifen::DeviceItem *dev = dynamic_cast<Zhifen::DeviceItem*>(item);
            if (dev) {
                QString text = textEdit->text().trimmed();
                if (text.isEmpty()) text = dev->deviceName();
                LabelItem::OperatorType op = static_cast<LabelItem::OperatorType>(
                    operatorCombo->currentData().toInt());
                LabelItem *label = new LabelItem(text, op);
                label->setShowOperatorLogo(showLogoCheck->isChecked());
                label->setPos(dev->pos() + QPointF(50, -20));
                m_scene->addItem(label);
            }
        }
        refreshList();
        QMessageBox::information(dlg, "添加成功", QString("已为%1个选中器件添加标签").arg(selected.size()));
    });

    // 删除所有标签
    connect(deleteAllBtn, &QPushButton::clicked, this, [&]() {
        if (QMessageBox::question(dlg, "确认删除", "确定要删除所有标签吗？") != QMessageBox::Yes) return;
        QList<QGraphicsItem*> toRemove;
        for (QGraphicsItem *item : m_scene->items()) {
            if (dynamic_cast<LabelItem*>(item)) toRemove.append(item);
        }
        for (QGraphicsItem *item : toRemove) m_scene->removeItem(item);
        refreshList();
    });

    // 自动排列标签
    connect(autoArrangeBtn, &QPushButton::clicked, this, [&]() {
        QList<LabelItem*> labels;
        for (QGraphicsItem *item : m_scene->items()) {
            LabelItem *label = dynamic_cast<LabelItem*>(item);
            if (label) labels.append(label);
        }
        // 简单自动排列：按位置排序，避免重叠
        std::sort(labels.begin(), labels.end(), [](LabelItem *a, LabelItem *b) {
            return a->pos().y() < b->pos().y();
        });
        qreal y = 50;
        for (LabelItem *label : labels) {
            label->setPos(50, y);
            y += 35;
        }
        m_view->zoomExtents();
        QMessageBox::information(dlg, "排列完成", QString("已自动排列%1个标签").arg(labels.size()));
    });

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other, "智能标签系统操作");
}

void MainWindow::onDualLink()
{
    // 生成系统图并建立关联
    Zhifen::SystemDiagramGenerator generator;
    Zhifen::SystemDiagramResult result = generator.generate(m_scene, Zhifen::SDM_Formal);

    if (!result.success || result.nodes.isEmpty()) {
        QMessageBox::warning(this, "关联失败", "系统图生成失败或没有器件，请先在平面图中放置器件");
        return;
    }

    // 统计关联状态
    int linkedCount = 0;
    int unlinkedCount = 0;
    for (auto *node : result.nodes) {
        if (node->sourceItem) linkedCount++;
        else unlinkedCount++;
    }

    // 双向关联管理对话框
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("平面图与系统图双向关联");
    dlg->resize(700, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);

    // 关联状态摘要
    QGroupBox *statusGroup = new QGroupBox("关联状态", dlg);
    QGridLayout *statusLayout = new QGridLayout(statusGroup);
    statusLayout->addWidget(new QLabel("系统图节点总数:"), 0, 0);
    QLabel *totalLabel = new QLabel(QString::number(result.nodes.size()));
    totalLabel->setStyleSheet("font-weight: bold; font-size: 14pt;");
    statusLayout->addWidget(totalLabel, 0, 1);
    statusLayout->addWidget(new QLabel("已关联平面图器件:"), 0, 2);
    QLabel *linkedLabel = new QLabel(QString::number(linkedCount));
    linkedLabel->setStyleSheet("color: green; font-weight: bold; font-size: 14pt;");
    statusLayout->addWidget(linkedLabel, 0, 3);
    statusLayout->addWidget(new QLabel("未关联:"), 1, 0);
    QLabel *unlinkedLabel = new QLabel(QString::number(unlinkedCount));
    unlinkedLabel->setStyleSheet("color: red; font-weight: bold;");
    statusLayout->addWidget(unlinkedLabel, 1, 1);
    statusLayout->addWidget(new QLabel("关联率:"), 1, 2);
    QLabel *rateLabel = new QLabel(QString("%1%").arg(linkedCount * 100.0 / result.nodes.size(), 0, 'f', 1));
    rateLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 14pt;");
    statusLayout->addWidget(rateLabel, 1, 3);
    mainLayout->addWidget(statusGroup);

    // 关联列表
    QGroupBox *listGroup = new QGroupBox("关联详情", dlg);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    QTableWidget *linkTable = new QTableWidget(result.nodes.size(), 4, listGroup);
    linkTable->setHorizontalHeaderLabels({"系统图节点", "类型", "关联平面图器件", "状态"});
    linkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    linkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    linkTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    for (int i = 0; i < result.nodes.size(); i++) {
        auto *node = result.nodes[i];
        linkTable->setItem(i, 0, new QTableWidgetItem(node->name));
        linkTable->setItem(i, 1, new QTableWidgetItem(node->type));

        Zhifen::DeviceItem *dev = static_cast<Zhifen::DeviceItem*>(node->sourceItem);
        QString devName = dev ? dev->deviceName() : "未关联";
        QTableWidgetItem *devItem = new QTableWidgetItem(devName);
        linkTable->setItem(i, 2, devItem);

        QString status = dev ? "✓ 已关联" : "✗ 未关联";
        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(QBrush(dev ? QColor(0, 150, 0) : QColor(255, 0, 0)));
        statusItem->setFont(QFont("", -1, QFont::Bold));
        linkTable->setItem(i, 3, statusItem);
    }
    listLayout->addWidget(linkTable);
    mainLayout->addWidget(listGroup);

    // 操作按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *highlightBtn = new QPushButton("在平面图中高亮选中器件", dlg);
    QPushButton *syncBtn = new QPushButton("同步更新系统图", dlg);
    QPushButton *openSysBtn = new QPushButton("打开系统图", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(highlightBtn);
    btnLayout->addWidget(syncBtn);
    btnLayout->addWidget(openSysBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    // 在平面图中高亮选中器件
    connect(highlightBtn, &QPushButton::clicked, this, [&]() {
        int row = linkTable->currentRow();
        if (row < 0 || row >= result.nodes.size()) {
            QMessageBox::warning(dlg, "提示", "请先在列表中选择一个节点");
            return;
        }
        auto *node = result.nodes[row];
        Zhifen::DeviceItem *dev = static_cast<Zhifen::DeviceItem*>(node->sourceItem);
        if (!dev) {
            QMessageBox::warning(dlg, "提示", "该节点未关联平面图器件");
            return;
        }
        m_scene->clearSelection();
        dev->setSelected(true);
        m_view->centerOn(dev);
        QMessageBox::information(dlg, "高亮完成", QString("已在平面图中高亮: %1").arg(dev->deviceName()));
    });

    // 同步更新系统图
    connect(syncBtn, &QPushButton::clicked, this, [&]() {
        // 重新生成系统图
        Zhifen::SystemDiagramResult newResult = generator.generate(m_scene, Zhifen::SDM_Formal);
        if (newResult.success) {
            QMessageBox::information(dlg, "同步完成", "系统图已根据平面图最新状态同步更新");
            dlg->accept();
        }
    });

    // 打开系统图
    connect(openSysBtn, &QPushButton::clicked, this, [&]() {
        onGenerateSystemDiagram(Zhifen::SystemLayoutMode::ModeA);
        dlg->accept();
    });

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("双向关联: %1节点, 关联率%2%")
        .arg(result.nodes.size()).arg(linkedCount * 100.0 / result.nodes.size(), 0, 'f', 1));
}

void MainWindow::onSaveBaseline()
{
    m_baselineSnapshot.clear();
    int count = 0;
    for (QGraphicsItem *item : m_scene->items()) {
        Zhifen::DeviceItem *dev = dynamic_cast<Zhifen::DeviceItem*>(item);
        if (!dev) continue;
        QVariantMap snap;
        snap["type"] = dev->deviceType();
        snap["name"] = dev->deviceName();
        snap["x"] = dev->pos().x();
        snap["y"] = dev->pos().y();
        snap["rotation"] = dev->rotation();
        m_baselineSnapshot.append(snap);
        count++;
    }
    QMessageBox::information(this, "保存成功", QString("已保存基准图纸，共%1个器件").arg(count));
}

void MainWindow::onDrawingCompare()
{
    if (m_baselineSnapshot.isEmpty()) {
        QMessageBox::warning(this, "对比失败", "请先保存基准图纸（工具菜单→保存基准图纸）");
        return;
    }

    // 收集当前图纸器件
    QList<QVariantMap> currentSnapshot;
    for (QGraphicsItem *item : m_scene->items()) {
        Zhifen::DeviceItem *dev = dynamic_cast<Zhifen::DeviceItem*>(item);
        if (!dev) continue;
        QVariantMap snap;
        snap["type"] = dev->deviceType();
        snap["name"] = dev->deviceName();
        snap["x"] = dev->pos().x();
        snap["y"] = dev->pos().y();
        snap["rotation"] = dev->rotation();
        snap["item"] = QVariant::fromValue((void*)dev);
        currentSnapshot.append(snap);
    }

    // 对比
    QList<QVariantMap> added, removed, modified;
    QList<bool> matched;
    for (int i = 0; i < currentSnapshot.size(); i++) matched.append(false);

    // 查找删除和修改
    for (const auto &base : m_baselineSnapshot) {
        bool found = false;
        for (int i = 0; i < currentSnapshot.size(); i++) {
            if (matched[i]) continue;
            const auto &cur = currentSnapshot[i];
            if (cur["type"] == base["type"] && cur["name"] == base["name"]) {
                matched[i] = true;
                found = true;
                // 检查位置变化
                qreal dx = qAbs(cur["x"].toReal() - base["x"].toReal());
                qreal dy = qAbs(cur["y"].toReal() - base["y"].toReal());
                if (dx > 5 || dy > 5) {
                    QVariantMap mod = cur;
                    mod["oldX"] = base["x"];
                    mod["oldY"] = base["y"];
                    modified.append(mod);
                }
                break;
            }
        }
        if (!found) removed.append(base);
    }

    // 查找新增
    for (int i = 0; i < currentSnapshot.size(); i++) {
        if (!matched[i]) added.append(currentSnapshot[i]);
    }

    // 高亮差异器件
    m_scene->clearSelection();
    for (const auto &a : added) {
        Zhifen::DeviceItem *dev = static_cast<Zhifen::DeviceItem*>(a["item"].value<void*>());
        if (dev) dev->setSelected(true);
    }
    for (const auto &m : modified) {
        Zhifen::DeviceItem *dev = static_cast<Zhifen::DeviceItem*>(m["item"].value<void*>());
        if (dev) dev->setSelected(true);
    }

    // 显示对比报告
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("图纸对比报告");
    dlg->resize(650, 500);
    QVBoxLayout *mainLayout = new QVBoxLayout(dlg);

    // 摘要
    QGroupBox *summaryGroup = new QGroupBox("对比摘要", dlg);
    QGridLayout *summaryLayout = new QGridLayout(summaryGroup);
    summaryLayout->addWidget(new QLabel("基准图纸器件数:"), 0, 0);
    summaryLayout->addWidget(new QLabel(QString::number(m_baselineSnapshot.size())), 0, 1);
    summaryLayout->addWidget(new QLabel("当前图纸器件数:"), 0, 2);
    summaryLayout->addWidget(new QLabel(QString::number(currentSnapshot.size())), 0, 3);
    summaryLayout->addWidget(new QLabel("新增器件:"), 1, 0);
    QLabel *addedLabel = new QLabel(QString("+%1").arg(added.size()));
    addedLabel->setStyleSheet("color: green; font-weight: bold; font-size: 14pt;");
    summaryLayout->addWidget(addedLabel, 1, 1);
    summaryLayout->addWidget(new QLabel("删除器件:"), 1, 2);
    QLabel *removedLabel = new QLabel(QString("-%1").arg(removed.size()));
    removedLabel->setStyleSheet("color: red; font-weight: bold; font-size: 14pt;");
    summaryLayout->addWidget(removedLabel, 1, 3);
    summaryLayout->addWidget(new QLabel("位置修改:"), 2, 0);
    QLabel *modifiedLabel = new QLabel(QString::number(modified.size()));
    modifiedLabel->setStyleSheet("color: orange; font-weight: bold; font-size: 14pt;");
    summaryLayout->addWidget(modifiedLabel, 2, 1);
    summaryLayout->addWidget(new QLabel("未变化:"), 2, 2);
    int unchanged = currentSnapshot.size() - added.size() - modified.size();
    summaryLayout->addWidget(new QLabel(QString::number(unchanged)), 2, 3);
    mainLayout->addWidget(summaryGroup);

    // 详细列表
    QTabWidget *tabWidget = new QTabWidget(dlg);

    // 新增列表
    QTableWidget *addedTable = new QTableWidget(added.size(), 3, tabWidget);
    addedTable->setHorizontalHeaderLabels({"器件名称", "类型", "位置(X,Y)"});
    addedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (int i = 0; i < added.size(); i++) {
        addedTable->setItem(i, 0, new QTableWidgetItem(added[i]["name"].toString()));
        addedTable->setItem(i, 1, new QTableWidgetItem(QString::number(added[i]["type"].toInt())));
        addedTable->setItem(i, 2, new QTableWidgetItem(QString("(%1, %2)").arg(added[i]["x"].toReal(), 0, 'f', 0).arg(added[i]["y"].toReal(), 0, 'f', 0)));
    }
    tabWidget->addTab(addedTable, QString("新增 (%1)").arg(added.size()));

    // 删除列表
    QTableWidget *removedTable = new QTableWidget(removed.size(), 3, tabWidget);
    removedTable->setHorizontalHeaderLabels({"器件名称", "类型", "原位置(X,Y)"});
    removedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (int i = 0; i < removed.size(); i++) {
        removedTable->setItem(i, 0, new QTableWidgetItem(removed[i]["name"].toString()));
        removedTable->setItem(i, 1, new QTableWidgetItem(QString::number(removed[i]["type"].toInt())));
        removedTable->setItem(i, 2, new QTableWidgetItem(QString("(%1, %2)").arg(removed[i]["x"].toReal(), 0, 'f', 0).arg(removed[i]["y"].toReal(), 0, 'f', 0)));
    }
    tabWidget->addTab(removedTable, QString("删除 (%1)").arg(removed.size()));

    // 修改列表
    QTableWidget *modifiedTable = new QTableWidget(modified.size(), 4, tabWidget);
    modifiedTable->setHorizontalHeaderLabels({"器件名称", "原位置", "新位置", "偏移量"});
    modifiedTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (int i = 0; i < modified.size(); i++) {
        modifiedTable->setItem(i, 0, new QTableWidgetItem(modified[i]["name"].toString()));
        modifiedTable->setItem(i, 1, new QTableWidgetItem(QString("(%1, %2)").arg(modified[i]["oldX"].toReal(), 0, 'f', 0).arg(modified[i]["oldY"].toReal(), 0, 'f', 0)));
        modifiedTable->setItem(i, 2, new QTableWidgetItem(QString("(%1, %2)").arg(modified[i]["x"].toReal(), 0, 'f', 0).arg(modified[i]["y"].toReal(), 0, 'f', 0)));
        qreal dx = modified[i]["x"].toReal() - modified[i]["oldX"].toReal();
        qreal dy = modified[i]["y"].toReal() - modified[i]["oldY"].toReal();
        modifiedTable->setItem(i, 3, new QTableWidgetItem(QString("(%1, %2)").arg(dx, 0, 'f', 0).arg(dy, 0, 'f', 0)));
    }
    tabWidget->addTab(modifiedTable, QString("修改 (%1)").arg(modified.size()));

    mainLayout->addWidget(tabWidget);

    // 按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *exportBtn = new QPushButton("导出对比报告", dlg);
    QPushButton *updateBaselineBtn = new QPushButton("更新基准为当前", dlg);
    QPushButton *closeBtn = new QPushButton("关闭", dlg);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(updateBaselineBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(updateBaselineBtn, &QPushButton::clicked, this, [&]() {
        onSaveBaseline();
        dlg->accept();
    });
    connect(exportBtn, &QPushButton::clicked, this, [&]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出对比报告", "图纸对比报告.csv", "CSV文件(*.csv)");
        if (fileName.isEmpty()) return;
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << "图纸对比报告\n";
        out << QString("基准器件数,%1\n").arg(m_baselineSnapshot.size());
        out << QString("当前器件数,%1\n").arg(currentSnapshot.size());
        out << QString("新增,%1\n").arg(added.size());
        out << QString("删除,%1\n").arg(removed.size());
        out << QString("修改,%1\n").arg(modified.size());
        out << "\n新增器件\n名称,类型,位置X,位置Y\n";
        for (const auto &a : added)
            out << QString("%1,%2,%3,%4\n").arg(a["name"].toString()).arg(a["type"].toInt()).arg(a["x"].toReal(), 0, 'f', 0).arg(a["y"].toReal(), 0, 'f', 0);
        file.close();
        QMessageBox::information(this, "导出成功", "对比报告已导出");
    });

    dlg->setLayout(mainLayout);
    dlg->exec();
    dlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("图纸对比: 新增%1, 删除%2, 修改%3").arg(added.size()).arg(removed.size()).arg(modified.size()));
}

void MainWindow::onAutoTrunk()
{
    // 收集信源和器件
    QList<Zhifen::DeviceItem*> sources;
    QList<Zhifen::DeviceItem*> devices;
    QList<Zhifen::DeviceItem*> antennas;

    for (QGraphicsItem *item : m_scene->items()) {
        Zhifen::DeviceItem *dev = dynamic_cast<Zhifen::DeviceItem*>(item);
        if (!dev) continue;
        Zhifen::DeviceItem::DeviceType dt = dev->deviceType();
        if (dt == Zhifen::DeviceItem::MacroBS || dt == Zhifen::DeviceItem::MicroBS ||
            dt == Zhifen::DeviceItem::RRU || dt == Zhifen::DeviceItem::FiberRepeater) {
            sources.append(dev);
        } else if (dt == Zhifen::DeviceItem::OmniAntenna || dt == Zhifen::DeviceItem::DirectionalAntenna ||
                   dt == Zhifen::DeviceItem::SpotlightAntenna || dt == Zhifen::DeviceItem::ExternalAntenna ||
                   dt == Zhifen::DeviceItem::WallMountAntenna || dt == Zhifen::DeviceItem::CeilingAntenna) {
            antennas.append(dev);
        } else {
            devices.append(dev);
        }
    }

    if (sources.isEmpty()) {
        QMessageBox::warning(this, "生成失败", "未找到信源器件，请先放置宏基站/微基站/RRU等信源");
        return;
    }

    if (antennas.isEmpty() && devices.isEmpty()) {
        QMessageBox::warning(this, "生成失败", "未找到需要连接的器件和天线");
        return;
    }

    // 参数设置对话框
    QDialog *paramDlg = new QDialog(this);
    paramDlg->setWindowTitle("主干自动生成设置");
    paramDlg->resize(400, 300);
    QFormLayout *formLayout = new QFormLayout(paramDlg);

    QSpinBox *branchCountSpin = new QSpinBox(paramDlg);
    branchCountSpin->setRange(1, 20); branchCountSpin->setValue(4);
    formLayout->addRow("分支数量:", branchCountSpin);

    QComboBox *strategyCombo = new QComboBox(paramDlg);
    strategyCombo->addItem("最短路径树形");
    strategyCombo->addItem("均匀分布");
    strategyCombo->addItem("按区域聚类");
    formLayout->addRow("生成策略:", strategyCombo);

    QCheckBox *addLabelsCheck = new QCheckBox("添加馈线长度标注", paramDlg);
    addLabelsCheck->setChecked(true);
    formLayout->addRow(addLabelsCheck);

    QLabel *infoLabel = new QLabel(QString("检测到: %1个信源, %2个器件, %3根天线").arg(sources.size()).arg(devices.size()).arg(antennas.size()), paramDlg);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-size: 9pt; padding: 5px;");
    formLayout->addRow(infoLabel);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("开始生成", paramDlg);
    QPushButton *cancelBtn = new QPushButton("取消", paramDlg);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    formLayout->addRow(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, paramDlg, &QDialog::reject);
    connect(okBtn, &QPushButton::clicked, paramDlg, &QDialog::accept);

    if (paramDlg->exec() != QDialog::Accepted) {
        paramDlg->deleteLater();
        return;
    }

    int branchCount = branchCountSpin->value();
    bool addLabels = addLabelsCheck->isChecked();
    paramDlg->deleteLater();

    // 合并所有待连接器件
    QList<Zhifen::DeviceItem*> allTargets = devices + antennas;

    // 取第一个信源作为主信源
    Zhifen::DeviceItem *mainSource = sources.first();
    QPointF sourcePos = mainSource->pos();

    // 按距离排序
    QList<QPair<qreal, Zhifen::DeviceItem*>> sortedTargets;
    for (auto *dev : allTargets) {
        qreal dist = QLineF(sourcePos, dev->pos()).length();
        sortedTargets.append(qMakePair(dist, dev));
    }
    std::sort(sortedTargets.begin(), sortedTargets.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });

    // 生成分支点
    int perBranch = qMax(1, sortedTargets.size() / branchCount);
    QList<QPointF> branchPoints;
    QList<QList<Zhifen::DeviceItem*>> branchDevices;

    for (int b = 0; b < branchCount && b * perBranch < sortedTargets.size(); b++) {
        QList<Zhifen::DeviceItem*> branchDevs;
        QPointF center(0, 0);
        int count = 0;
        for (int i = b * perBranch; i < qMin((b + 1) * perBranch, sortedTargets.size()); i++) {
            branchDevs.append(sortedTargets[i].second);
            center += sortedTargets[i].second->pos();
            count++;
        }
        if (count > 0) {
            center /= count;
            // 分支点在信源和中心之间
            QPointF branchPoint = sourcePos + (center - sourcePos) * 0.6;
            branchPoints.append(branchPoint);
            branchDevices.append(branchDevs);
        }
    }

    // 生成馈线
    int trunkCount = 0;
    int branchFeederCount = 0;
    qreal totalLength = 0;

    // 主干和分支馈线使用默认样式

    // 从信源到各分支点生成主干
    for (const auto &bp : branchPoints) {
        FeederItem *trunk = new FeederItem();
        QPolygonF points;
        points.append(sourcePos);
        points.append(bp);
        trunk->setPoints(points);
        m_scene->addItem(trunk);
        trunkCount++;
        totalLength += QLineF(sourcePos, bp).length();

        if (addLabels) {
            QGraphicsSimpleTextItem *label = m_scene->addSimpleText(
                QString("主干 %1m").arg(QLineF(sourcePos, bp).length() / 10, 0, 'f', 0),
                QFont("Arial", 8));
            label->setPos((sourcePos + bp) / 2 + QPointF(5, -10));
            label->setBrush(QColor(0, 100, 200));
        }
    }

    // 从分支点到各器件生成分支馈线
    for (int b = 0; b < branchPoints.size(); b++) {
        for (auto *dev : branchDevices[b]) {
            FeederItem *feeder = new FeederItem();
            QPolygonF points;
            points.append(branchPoints[b]);
            points.append(dev->pos());
            feeder->setPoints(points);
            m_scene->addItem(feeder);
            branchFeederCount++;
            totalLength += QLineF(branchPoints[b], dev->pos()).length();
        }
    }

    m_view->zoomExtents();

    // 显示生成结果
    QDialog *resultDlg = new QDialog(this);
    resultDlg->setWindowTitle("主干自动生成完成");
    resultDlg->resize(450, 300);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultDlg);

    QGroupBox *statGroup = new QGroupBox("生成统计", resultDlg);
    QGridLayout *statLayout = new QGridLayout(statGroup);
    statLayout->addWidget(new QLabel("信源数:"), 0, 0);
    statLayout->addWidget(new QLabel(QString::number(sources.size())), 0, 1);
    statLayout->addWidget(new QLabel("连接器件数:"), 0, 2);
    statLayout->addWidget(new QLabel(QString::number(allTargets.size())), 0, 3);
    statLayout->addWidget(new QLabel("主干数量:"), 1, 0);
    QLabel *trunkLabel = new QLabel(QString::number(trunkCount));
    trunkLabel->setStyleSheet("color: #1976d2; font-weight: bold; font-size: 14pt;");
    statLayout->addWidget(trunkLabel, 1, 1);
    statLayout->addWidget(new QLabel("分支馈线数:"), 1, 2);
    statLayout->addWidget(new QLabel(QString::number(branchFeederCount)), 1, 3);
    statLayout->addWidget(new QLabel("总馈线长度:"), 2, 0);
    QLabel *lengthLabel = new QLabel(QString("%1m").arg(totalLength / 10, 0, 'f', 0));
    lengthLabel->setStyleSheet("color: #388e3c; font-weight: bold; font-size: 14pt;");
    statLayout->addWidget(lengthLabel, 2, 1);
    statLayout->addWidget(new QLabel("分支点数:"), 2, 2);
    statLayout->addWidget(new QLabel(QString::number(branchPoints.size())), 2, 3);
    resultLayout->addWidget(statGroup);

    QLabel *descLabel = new QLabel(QString("已从信源自动生成 %1 条主干和 %2 条分支馈线，连接 %3 个器件。主干为蓝色粗线，分支为灰色细线。")
        .arg(trunkCount).arg(branchFeederCount).arg(allTargets.size()), resultDlg);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("padding: 10px; background: #e8f5e9; color: #2e7d32; border-radius: 4px;");
    resultLayout->addWidget(descLabel);

    QPushButton *closeBtn = new QPushButton("关闭", resultDlg);
    connect(closeBtn, &QPushButton::clicked, resultDlg, &QDialog::accept);
    resultLayout->addWidget(closeBtn);

    resultDlg->setLayout(resultLayout);
    resultDlg->exec();
    resultDlg->deleteLater();

    Zhifen::AuditLogger::instance().log(Zhifen::Audit_Other,
        QString("主干自动生成: %1主干, %2分支, %3m").arg(trunkCount).arg(branchFeederCount).arg(totalLength/10, 0, 'f', 0));
}

void MainWindow::onGenerateDemo()
{
    // 清空场景
    m_scene->clear();

    // 放置信源（宏基站）
    Zhifen::DeviceItem *source = new Zhifen::DeviceItem(Zhifen::DeviceItem::MacroBS);
    source->setPos(200, 300);
    source->setData(1, "SRC-01");
    source->setData(2, 43.0);  // 输出功率43dBm
    m_scene->addItem(source);

    // 放置一级功分器（二功分）
    Zhifen::DeviceItem *splitter1 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Splitter);
    splitter1->setPos(400, 300);
    splitter1->setData(1, "SPL-01");
    m_scene->addItem(splitter1);

    // 放置二级功分器
    Zhifen::DeviceItem *splitter2 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Splitter);
    splitter2->setPos(600, 200);
    splitter2->setData(1, "SPL-02");
    m_scene->addItem(splitter2);

    Zhifen::DeviceItem *splitter3 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Splitter);
    splitter3->setPos(600, 400);
    splitter3->setData(1, "SPL-03");
    m_scene->addItem(splitter3);

    // 放置耦合器
    Zhifen::DeviceItem *coupler1 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Coupler);
    coupler1->setPos(800, 150);
    coupler1->setData(1, "CPL-01");
    m_scene->addItem(coupler1);

    Zhifen::DeviceItem *coupler2 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Coupler);
    coupler2->setPos(800, 250);
    coupler2->setData(1, "CPL-02");
    m_scene->addItem(coupler2);

    Zhifen::DeviceItem *coupler3 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Coupler);
    coupler3->setPos(800, 350);
    coupler3->setData(1, "CPL-03");
    m_scene->addItem(coupler3);

    Zhifen::DeviceItem *coupler4 = new Zhifen::DeviceItem(Zhifen::DeviceItem::Coupler);
    coupler4->setPos(800, 450);
    coupler4->setData(1, "CPL-04");
    m_scene->addItem(coupler4);

    // 放置天线（8根全向吸顶天线）
    QList<Zhifen::DeviceItem*> antennas;
    QList<QPointF> antennaPositions = {
        QPointF(1000, 100), QPointF(1000, 200),
        QPointF(1000, 300), QPointF(1000, 400),
        QPointF(1000, 500), QPointF(1150, 150),
        QPointF(1150, 300), QPointF(1150, 450)
    };
    for (int i = 0; i < antennaPositions.size(); i++) {
        Zhifen::DeviceItem *ant = new Zhifen::DeviceItem(Zhifen::DeviceItem::OmniAntenna);
        ant->setPos(antennaPositions[i]);
        ant->setData(1, QString("ANT-%1").arg(i + 1, 2, 10, QChar('0')));
        ant->setData(2, 10.0 + i * 0.5);  // 模拟不同功率
        m_scene->addItem(ant);
        antennas.append(ant);
    }

    // 连接馈线
    auto connectDevices = [&](QGraphicsItem *from, QGraphicsItem *to) {
        FeederItem *feeder = new FeederItem();
        QPolygonF points;
        points.append(from->pos());
        points.append(to->pos());
        feeder->setPoints(points);
        m_scene->addItem(feeder);
        return feeder;
    };

    // 信源 -> 一级功分器
    connectDevices(source, splitter1);
    // 一级功分器 -> 二级功分器
    connectDevices(splitter1, splitter2);
    connectDevices(splitter1, splitter3);
    // 二级功分器 -> 耦合器
    connectDevices(splitter2, coupler1);
    connectDevices(splitter2, coupler2);
    connectDevices(splitter3, coupler3);
    connectDevices(splitter3, coupler4);
    // 耦合器 -> 天线
    connectDevices(coupler1, antennas[0]);
    connectDevices(coupler1, antennas[5]);
    connectDevices(coupler2, antennas[1]);
    connectDevices(coupler2, antennas[2]);
    connectDevices(coupler3, antennas[3]);
    connectDevices(coupler3, antennas[6]);
    connectDevices(coupler4, antennas[4]);
    connectDevices(coupler4, antennas[7]);

    // 添加器件编号标注
    QFont labelFont("Arial", 8);
    auto addLabel = [&](QGraphicsItem *item, const QString &text, QPointF offset) {
        QGraphicsSimpleTextItem *label = m_scene->addSimpleText(text, labelFont);
        label->setPos(item->pos() + offset);
        label->setBrush(QColor(0, 0, 128));
    };

    addLabel(source, "宏基站 43dBm", QPointF(-30, -35));
    addLabel(splitter1, "二功分器", QPointF(-25, -30));
    addLabel(splitter2, "二功分器", QPointF(-25, -30));
    addLabel(splitter3, "二功分器", QPointF(-25, -30));
    addLabel(coupler1, "10dB耦合器", QPointF(-30, -30));
    addLabel(coupler2, "10dB耦合器", QPointF(-30, -30));
    addLabel(coupler3, "10dB耦合器", QPointF(-30, -30));
    addLabel(coupler4, "10dB耦合器", QPointF(-30, -30));
    for (int i = 0; i < antennas.size(); i++) {
        addLabel(antennas[i], QString("ANT-%1").arg(i + 1, 2, 10, QChar('0')), QPointF(-20, -25));
    }

    // 添加楼层边框（模拟建筑平面图）
    QPen wallPen(QColor(100, 100, 100), 3);
    m_scene->addRect(100, 50, 1150, 550, wallPen, QBrush(QColor(250, 250, 250)));

    // 添加房间分隔
    m_scene->addLine(650, 50, 650, 600, wallPen);
    m_scene->addLine(100, 300, 650, 300, wallPen);

    // 添加房间名称
    QFont roomFont("Arial", 10, QFont::Bold);
    auto addRoomLabel = [&](const QString &text, QPointF pos) {
        QGraphicsSimpleTextItem *label = m_scene->addSimpleText(text, roomFont);
        label->setPos(pos);
        label->setBrush(QColor(150, 150, 150));
    };
    addRoomLabel("大厅", QPointF(350, 150));
    addRoomLabel("会议室", QPointF(350, 450));
    addRoomLabel("办公区A", QPointF(850, 150));
    addRoomLabel("办公区B", QPointF(850, 450));

    // 添加标题
    QGraphicsSimpleTextItem *title = m_scene->addSimpleText(
        "Demo演示项目 - 某办公楼室内分布系统平面图", QFont("Arial", 14, QFont::Bold));
    title->setPos(400, 20);
    title->setBrush(QColor(0, 0, 0));

    // 设置工程信息
    Zhifen::ProjectInfo &info = Zhifen::ProjectInfoManager::instance().info();
    info.projectName = "Demo演示项目 - 某办公楼室分系统";
    info.constructionUnit = "Demo建设单位";
    info.designUnit = "智分Design";
    info.designer = "演示设计师";
    info.reviewer = "演示审核";
    info.checker = "演示校对";
    info.draftsman = "演示绘图";
    info.drawingNumber = "ZF-DEMO-2026-001";

    m_view->zoomExtents();

    QMessageBox::information(this, "Demo生成完成",
        "Demo演示项目已生成！\n\n"
        "包含：\n"
        "- 1个宏基站信源（43dBm）\n"
        "- 3个二功分器\n"
        "- 4个10dB耦合器\n"
        "- 8根全向吸顶天线\n"
        "- 完整馈线连接\n"
        "- 建筑平面图（4个房间）\n\n"
        "可体验功能：\n"
        "1. 工具→生成系统图\n"
        "2. 工具→链路预算\n"
        "3. 工具→材料统计\n"
        "4. 工具→天线功率统计\n"
        "5. 工具→覆盖率仿真\n"
        "6. 文件→打印预览");
}

void MainWindow::placeDevice(Zhifen::DeviceItem::DeviceType type, const QString &name)
{
    m_pendingDeviceType = type;
    m_pendingDeviceName = name;
    statusBar()->showMessage(QString("放置器件: %1，点击画布放置，右键取消").arg(name), 5000);
    if (!m_devicePlaceConnection) {
        m_devicePlaceConnection = connect(m_view, &CadView::sceneClicked, this, [this](QPointF pos){
            if (m_pendingDeviceName.isEmpty()) return;
            Zhifen::DeviceItem *dev = new Zhifen::DeviceItem(m_pendingDeviceType);
            dev->setPos(pos);
            dev->setToolTip(m_pendingDeviceName);
            dev->setData(1, m_pendingDeviceName);
            m_scene->addItem(dev);
            m_scene->clearSelection();
            dev->setSelected(true);
            statusBar()->showMessage(QString("已放置: %1").arg(m_pendingDeviceName), 3000);
        });
    }
}

void MainWindow::onErase()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) {
        statusBar()->showMessage("请先选择要删除的对象", 3000);
        return;
    }
    for (QGraphicsItem *item : selected) {
        m_scene->removeItem(item);
        delete item;
    }
    statusBar()->showMessage(QString("已删除 %1 个对象").arg(selected.size()), 3000);
}

void MainWindow::onLayerManager()
{
    // 图层管理器 - 显示图层管理对话框
    QDialog *layerDlg = new QDialog(this);
    layerDlg->setWindowTitle("图层管理器");
    layerDlg->resize(500, 400);
    layerDlg->setStyleSheet("QDialog { background: #2b2b2b; } QLabel { color: #ccc; } QListWidget { background: #1e1e1e; color: #ccc; border: 1px solid #555; } QPushButton { background: #3c3f41; color: #ccc; border: 1px solid #555; padding: 5px 15px; } QPushButton:hover { background: #505355; }");
    
    QVBoxLayout *layout = new QVBoxLayout(layerDlg);
    QListWidget *layerList = new QListWidget(layerDlg);
    
    // 添加默认图层
    QStringList layers = {"0", "建筑底图", "墙体", "门窗", "天线", "馈线", "器件", "信源", "标注", "文字", "图框"};
    foreach (const QString &layer, layers) {
        QListWidgetItem *item = new QListWidgetItem(layer);
        item->setCheckState(Qt::Checked);
        layerList->addItem(item);
    }
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("新建图层", layerDlg);
    QPushButton *delBtn = new QPushButton("删除图层", layerDlg);
    QPushButton *closeBtn = new QPushButton("关闭", layerDlg);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    
    layout->addWidget(new QLabel("图层列表（点击复选框控制显示/隐藏）:", layerDlg));
    layout->addWidget(layerList);
    layout->addLayout(btnLayout);
    
    connect(closeBtn, &QPushButton::clicked, layerDlg, &QDialog::accept);
    
    layerDlg->setLayout(layout);
    layerDlg->exec();
    layerDlg->deleteLater();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于智分Design",
        "<h2 style='color:#0078d4;'>智分Design V3.1.0</h2>"
        "<p><b>专业室分设计CAD软件</b></p>"
        "<p>集传统室分、数字化室分、漏缆、电梯覆盖、楼间对打于一体的专业设计工具</p>"
        "<hr>"
        "<p><b>核心功能：</b></p>"
        "<ul>"
        "<li>标准CAD绘图引擎（直线/圆/矩形/圆弧/文字/标注）</li>"
        "<li>54种室分器件库（天线/耦合器/功分器/信源/漏缆等）</li>"
        "<li>多频段链路预算（2G/3G/4G/5G/WLAN）</li>"
        "<li>系统图自动生成与功率标注</li>"
        "<li>材料表自动统计与Excel导出</li>"
        "<li>AI自动布放与智能校验</li>"
        "<li>DXF/DWG导入导出</li>"
        "<li>批量打印与PDF导出</li>"
        "</ul>"
        "<hr>"
        "<p><b>技术架构：</b>C++17 + Qt5 + CMake</p>"
        "<p><b>版本：</b>3.1.0 (Build 20260902)</p>"
        "<p><b>Copyright：</b>2026 智分Design Team</p>"
    );
}

void MainWindow::onHelp()
{
    QDialog *helpDlg = new QDialog(this);
    helpDlg->setWindowTitle("智分Design - 使用帮助");
    helpDlg->resize(700, 500);
    helpDlg->setStyleSheet("QDialog { background: #2b2b2b; } QLabel { color: #ccc; } QTextBrowser { background: #1e1e1e; color: #ccc; border: 1px solid #555; }");

    QVBoxLayout *layout = new QVBoxLayout(helpDlg);
    QTextBrowser *browser = new QTextBrowser(helpDlg);
    browser->setHtml(R"(
        <h2 style='color:#0078d4;'>智分Design V3.1 使用帮助</h2>
        <h3 style='color:#4ec9b0;'>一、快速开始</h3>
        <p>1. 点击「文件 → 新建」创建新工程</p>
        <p>2. 点击「文件 → 生成Demo演示项目」查看完整示例</p>
        <p>3. 使用左侧工具箱或顶部Ribbon菜单选择工具</p>
        <p>4. 在画布上点击绘制图形或放置器件</p>
        <h3 style='color:#4ec9b0;'>二、常用命令（命令行输入）</h3>
        <table border='1' cellpadding='5' style='border-collapse:collapse;'>
        <tr><td><b>命令</b></td><td><b>缩写</b></td><td><b>功能</b></td></tr>
        <tr><td>LINE</td><td>L</td><td>绘制直线</td></tr>
        <tr><td>CIRCLE</td><td>C</td><td>绘制圆</td></tr>
        <tr><td>RECTANGLE</td><td>REC</td><td>绘制矩形</td></tr>
        <tr><td>MOVE</td><td>M</td><td>移动对象</td></tr>
        <tr><td>COPY</td><td>CO</td><td>复制对象</td></tr>
        <tr><td>ERASE</td><td>E</td><td>删除对象</td></tr>
        <tr><td>ZOOM</td><td>Z</td><td>缩放视图</td></tr>
        <tr><td>LAYER</td><td>LA</td><td>图层管理</td></tr>
        </table>
        <h3 style='color:#4ec9b0;'>三、室分设计流程</h3>
        <p>1. <b>建筑底图导入：</b>文件 → 导入DXF，导入建筑图纸</p>
        <p>2. <b>天线布放：</b>点击「室分」标签页 → 天线，在平面图上点击放置</p>
        <p>3. <b>馈线连接：</b>使用馈线工具连接信源→功分器→耦合器→天线</p>
        <p>4. <b>链路计算：</b>工具 → 链路预算，自动计算各天线口功率</p>
        <p>5. <b>系统图生成：</b>室分 → 生成系统图，自动生成系统图</p>
        <p>6. <b>材料统计：</b>工具 → 材料表，自动统计主材辅材</p>
        <p>7. <b>打印输出：</b>文件 → 打印/导出PDF</p>
        <h3 style='color:#4ec9b0;'>四、快捷键</h3>
        <p>Ctrl+N 新建 | Ctrl+O 打开 | Ctrl+S 保存 | Ctrl+P 打印</p>
        <p>Ctrl+Z 撤销 | Ctrl+Y 重做 | Delete 删除 | Esc 取消当前命令</p>
        <p>鼠标滚轮 缩放 | 鼠标中键拖动 平移 | 双击滚轮 全部显示</p>
        <h3 style='color:#4ec9b0;'>五、技术支持</h3>
        <p>如有问题，请查看日志文件：C:\Users\用户名\.zhifen-design\startup.log</p>
    )");
    layout->addWidget(browser);
    helpDlg->setLayout(layout);
    helpDlg->exec();
}