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
    m_exportDxfAct = new QAction("导出DXF", this); connect(m_exportDxfAct, &QAction::triggered, this, &MainWindow::onExportDxf);
    m_printAct = new QAction("打印", this); m_printAct->setShortcut(QKeySequence::Print); connect(m_printAct, &QAction::triggered, this, &MainWindow::onPrint);
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
    m_textAct = new QAction("文字", this); m_textAct->setCheckable(true); m_toolGroup->addAction(m_textAct); connect(m_textAct, &QAction::triggered, this, [this](){ setCurrentTool("text"); });
    m_dimensionAct = new QAction("标注", this); m_dimensionAct->setCheckable(true); m_toolGroup->addAction(m_dimensionAct); connect(m_dimensionAct, &QAction::triggered, this, [this](){ setCurrentTool("dimension"); });

    m_moveAct = new QAction("移动", this); m_moveAct->setShortcut(Qt::Key_M); connect(m_moveAct, &QAction::triggered, this, [this](){ setCurrentTool("move"); });
    m_copyAct = new QAction("复制", this); connect(m_copyAct, &QAction::triggered, this, [this](){ setCurrentTool("copy"); });
    m_rotateAct = new QAction("旋转", this); connect(m_rotateAct, &QAction::triggered, this, [this](){ setCurrentTool("rotate"); });
    m_scaleAct = new QAction("缩放", this); connect(m_scaleAct, &QAction::triggered, this, [this](){ setCurrentTool("scale"); });
    m_eraseAct = new QAction("删除", this); m_eraseAct->setShortcut(Qt::Key_E); connect(m_eraseAct, &QAction::triggered, this, [this](){ auto items = m_scene->selectedItems(); if(!items.isEmpty()) m_undoStack->push(new RemoveItemsCommand(m_scene, items)); });

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
    fileMenu->addAction(m_importDxfAct); fileMenu->addAction(m_exportDxfAct); fileMenu->addSeparator();
    fileMenu->addAction(m_printAct); fileMenu->addSeparator(); fileMenu->addAction(m_exitAct);

    QMenu *editMenu = menuBar()->addMenu("编辑");
    editMenu->addAction(m_undoAct); editMenu->addAction(m_redoAct); editMenu->addSeparator();
    editMenu->addAction(m_moveAct); editMenu->addAction(m_copyAct); editMenu->addAction(m_rotateAct);
    editMenu->addAction(m_scaleAct); editMenu->addAction(m_eraseAct);

    QMenu *drawMenu = menuBar()->addMenu("绘图");
    drawMenu->addAction(m_lineAct); drawMenu->addAction(m_circleAct); drawMenu->addAction(m_arcAct);
    drawMenu->addAction(m_polylineAct); drawMenu->addAction(m_rectangleAct); drawMenu->addSeparator();
    drawMenu->addAction(m_textAct); drawMenu->addAction(m_dimensionAct);

    QMenu *viewMenu = menuBar()->addMenu("视图");
    viewMenu->addAction(m_panAct); viewMenu->addAction(m_zoomAct); viewMenu->addAction(m_zoomExtentsAct);
    viewMenu->addSeparator(); viewMenu->addAction(m_gridAct); viewMenu->addAction(m_snapAct); viewMenu->addAction(m_orthoAct);

    QMenu *toolsMenu = menuBar()->addMenu("工具");
    toolsMenu->addAction(m_layerManagerAct);

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
    m_drawToolBar->addSeparator();
    m_drawToolBar->addAction(m_textAct);
    m_drawToolBar->addAction(m_dimensionAct);

    m_editToolBar = addToolBar("编辑");
    m_editToolBar->setMovable(false);
    m_editToolBar->addAction(m_undoAct);
    m_editToolBar->addAction(m_redoAct);
    m_editToolBar->addSeparator();
    m_editToolBar->addAction(m_moveAct);
    m_editToolBar->addAction(m_copyAct);
    m_editToolBar->addAction(m_rotateAct);
    m_editToolBar->addAction(m_scaleAct);
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
    else if (toolName == "text") m_currentTool = new TextTool(m_view);
    else if (toolName == "dimension") m_currentTool = new DimensionTool(m_view);
    else if (toolName == "copy") m_currentTool = new CopyTool(m_view);
    else if (toolName == "rotate") m_currentTool = new RotateTool(m_view);
    else if (toolName == "scale") m_currentTool = new ScaleTool(m_view);
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

void MainWindow::onUndo() { m_undoStack->undo(); }
void MainWindow::onRedo() { m_undoStack->redo(); }

void MainWindow::onZoomExtents() { m_view->zoomExtents(); }
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
