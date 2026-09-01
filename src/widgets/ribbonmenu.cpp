#include "ribbonmenu.h"
#include "mainwindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QIcon>
#include <QFont>

namespace Zhifen {

RibbonMenu::RibbonMenu(QWidget *parent)
    : QTabWidget(parent)
{
    setTabPosition(QTabWidget::North);
    setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #3c3c3c; background: #252526; }
        QTabBar::tab { background: #2d2d30; color: #ccc; padding: 8px 20px; border: 1px solid #3c3c3c; border-bottom: none; margin-right: 2px; }
        QTabBar::tab:selected { background: #252526; color: #fff; border-bottom: 2px solid #0e639c; }
        QTabBar::tab:hover { background: #3c3c3c; }
        QGroupBox { color: #999; border: none; border-right: 1px solid #3c3c3c; margin-top: 4px; padding-top: 4px; font-size: 10px; }
        QGroupBox::title { subcontrol-origin: margin; left: 4px; padding: 0 4px; }
        QToolButton { background: transparent; color: #ccc; border: none; padding: 4px 8px; font-size: 11px; }
        QToolButton:hover { background: #3c3c3c; border-radius: 3px; }
        QToolButton:pressed { background: #0e639c; }
        QLabel { color: #ccc; font-size: 11px; }
    )");

    addTab(createHomeTab(), "常用");
    addTab(createDrawTab(), "绘图");
    addTab(createEditTab(), "编辑");
    addTab(createIndoorTab(), "室分");
    addTab(createDimensionTab(), "标注");
    addTab(createToolsTab(), "工具");
    addTab(createViewTab(), "视图");
    addTab(createOutputTab(), "输出");
}

QWidget *RibbonMenu::createGroup(const QString &title, QWidget *parent)
{
    QGroupBox *group = new QGroupBox(title, parent);
    group->setAlignment(Qt::AlignHCenter);
    QVBoxLayout *layout = new QVBoxLayout(group);
    layout->setSpacing(2);
    layout->setContentsMargins(4, 12, 4, 4);
    return group;
}

QToolBar *RibbonMenu::createToolBar(QWidget *parent)
{
    QToolBar *bar = new QToolBar(parent);
    bar->setIconSize(QSize(24, 24));
    bar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    bar->setMovable(false);
    bar->setFloatable(false);
    return bar;
}

QWidget *RibbonMenu::createHomeTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    // 绘图组
    QWidget *drawGroup = createGroup("绘图", tab);
    QToolBar *drawBar = createToolBar(drawGroup);
    m_actionLine = drawBar->addAction("直线");
    m_actionCircle = drawBar->addAction("圆");
    m_actionRect = drawBar->addAction("矩形");
    m_actionArc = drawBar->addAction("圆弧");
    m_actionText = drawBar->addAction("文字");
    static_cast<QVBoxLayout*>(drawGroup->layout())->addWidget(drawBar);
    layout->addWidget(drawGroup);

    // 编辑组
    QWidget *editGroup = createGroup("编辑", tab);
    QToolBar *editBar = createToolBar(editGroup);
    m_actionMove = editBar->addAction("移动");
    m_actionCopy = editBar->addAction("复制");
    m_actionRotate = editBar->addAction("旋转");
    m_actionScale = editBar->addAction("缩放");
    m_actionMirror = editBar->addAction("镜像");
    m_actionDelete = editBar->addAction("删除");
    static_cast<QVBoxLayout*>(editGroup->layout())->addWidget(editBar);
    layout->addWidget(editGroup);

    // 撤销重做组
    QWidget *undoGroup = createGroup("撤销", tab);
    QToolBar *undoBar = createToolBar(undoGroup);
    m_actionUndo = undoBar->addAction("撤销");
    m_actionRedo = undoBar->addAction("重做");
    static_cast<QVBoxLayout*>(undoGroup->layout())->addWidget(undoBar);
    layout->addWidget(undoGroup);

    // 图层组
    QWidget *layerGroup = createGroup("图层", tab);
    QToolBar *layerBar = createToolBar(layerGroup);
    m_actionLayer = layerBar->addAction("图层");
    static_cast<QVBoxLayout*>(layerGroup->layout())->addWidget(layerBar);
    layout->addWidget(layerGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createDrawTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *lineGroup = createGroup("直线", tab);
    QToolBar *lineBar = createToolBar(lineGroup);
    lineBar->addAction("直线");
    lineBar->addAction("构造线");
    lineBar->addAction("多段线");
    static_cast<QVBoxLayout*>(lineGroup->layout())->addWidget(lineBar);
    layout->addWidget(lineGroup);

    QWidget *circleGroup = createGroup("曲线", tab);
    QToolBar *circleBar = createToolBar(circleGroup);
    circleBar->addAction("圆");
    circleBar->addAction("圆弧");
    circleBar->addAction("椭圆");
    static_cast<QVBoxLayout*>(circleGroup->layout())->addWidget(circleBar);
    layout->addWidget(circleGroup);

    QWidget *polyGroup = createGroup("多边形", tab);
    QToolBar *polyBar = createToolBar(polyGroup);
    polyBar->addAction("矩形");
    polyBar->addAction("多边形");
    static_cast<QVBoxLayout*>(polyGroup->layout())->addWidget(polyBar);
    layout->addWidget(polyGroup);

    QWidget *textGroup = createGroup("文字", tab);
    QToolBar *textBar = createToolBar(textGroup);
    textBar->addAction("单行文字");
    textBar->addAction("多行文字");
    static_cast<QVBoxLayout*>(textGroup->layout())->addWidget(textBar);
    layout->addWidget(textGroup);

    QWidget *hatchGroup = createGroup("填充", tab);
    QToolBar *hatchBar = createToolBar(hatchGroup);
    hatchBar->addAction("图案填充");
    hatchBar->addAction("渐变色");
    static_cast<QVBoxLayout*>(hatchGroup->layout())->addWidget(hatchBar);
    layout->addWidget(hatchGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createEditTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *modifyGroup = createGroup("修改", tab);
    QToolBar *modifyBar = createToolBar(modifyGroup);
    modifyBar->addAction("移动");
    modifyBar->addAction("复制");
    modifyBar->addAction("旋转");
    modifyBar->addAction("缩放");
    modifyBar->addAction("镜像");
    modifyBar->addAction("删除");
    static_cast<QVBoxLayout*>(modifyGroup->layout())->addWidget(modifyBar);
    layout->addWidget(modifyGroup);

    QWidget *trimGroup = createGroup("修剪", tab);
    QToolBar *trimBar = createToolBar(trimGroup);
    trimBar->addAction("修剪");
    trimBar->addAction("延伸");
    trimBar->addAction("打断");
    trimBar->addAction("合并");
    static_cast<QVBoxLayout*>(trimGroup->layout())->addWidget(trimBar);
    layout->addWidget(trimGroup);

    QWidget *offsetGroup = createGroup("偏移", tab);
    QToolBar *offsetBar = createToolBar(offsetGroup);
    offsetBar->addAction("偏移");
    offsetBar->addAction("阵列");
    static_cast<QVBoxLayout*>(offsetGroup->layout())->addWidget(offsetBar);
    layout->addWidget(offsetGroup);

    QWidget *clipboardGroup = createGroup("剪贴板", tab);
    QToolBar *clipboardBar = createToolBar(clipboardGroup);
    clipboardBar->addAction("剪切");
    clipboardBar->addAction("复制");
    clipboardBar->addAction("粘贴");
    static_cast<QVBoxLayout*>(clipboardGroup->layout())->addWidget(clipboardBar);
    layout->addWidget(clipboardGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createIndoorTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *sourceGroup = createGroup("信源", tab);
    QToolBar *sourceBar = createToolBar(sourceGroup);
    m_actionSource = sourceBar->addAction("信源");
    sourceBar->addAction("光纤直放站");
    sourceBar->addAction("BBU");
    static_cast<QVBoxLayout*>(sourceGroup->layout())->addWidget(sourceBar);
    layout->addWidget(sourceGroup);

    QWidget *antennaGroup = createGroup("天线", tab);
    QToolBar *antennaBar = createToolBar(antennaGroup);
    m_actionAntenna = antennaBar->addAction("全向天线");
    antennaBar->addAction("定向天线");
    antennaBar->addAction("射灯天线");
    antennaBar->addAction("外引天线");
    static_cast<QVBoxLayout*>(antennaGroup->layout())->addWidget(antennaBar);
    layout->addWidget(antennaGroup);

    QWidget *deviceGroup = createGroup("器件", tab);
    QToolBar *deviceBar = createToolBar(deviceGroup);
    m_actionCoupler = deviceBar->addAction("耦合器");
    m_actionSplitter = deviceBar->addAction("功分器");
    deviceBar->addAction("合路器");
    deviceBar->addAction("电桥");
    deviceBar->addAction("衰减器");
    deviceBar->addAction("负载");
    static_cast<QVBoxLayout*>(deviceGroup->layout())->addWidget(deviceBar);
    layout->addWidget(deviceGroup);

    QWidget *feederGroup = createGroup("馈线", tab);
    QToolBar *feederBar = createToolBar(feederGroup);
    m_actionFeeder = feederBar->addAction("馈线");
    feederBar->addAction("跳线");
    feederBar->addAction("漏缆");
    static_cast<QVBoxLayout*>(feederGroup->layout())->addWidget(feederBar);
    layout->addWidget(feederGroup);

    QWidget *calcGroup = createGroup("计算", tab);
    QToolBar *calcBar = createToolBar(calcGroup);
    calcBar->addAction("链路预算");
    calcBar->addAction("功率平衡");
    calcBar->addAction("材料统计");
    static_cast<QVBoxLayout*>(calcGroup->layout())->addWidget(calcBar);
    layout->addWidget(calcGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createDimensionTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *dimGroup = createGroup("标注", tab);
    QToolBar *dimBar = createToolBar(dimGroup);
    m_actionDimLinear = dimBar->addAction("线性");
    m_actionDimAligned = dimBar->addAction("对齐");
    dimBar->addAction("半径");
    dimBar->addAction("直径");
    dimBar->addAction("角度");
    static_cast<QVBoxLayout*>(dimGroup->layout())->addWidget(dimBar);
    layout->addWidget(dimGroup);

    QWidget *textGroup = createGroup("文字", tab);
    QToolBar *textBar = createToolBar(textGroup);
    textBar->addAction("单行文字");
    textBar->addAction("多行文字");
    textBar->addAction("编辑文字");
    static_cast<QVBoxLayout*>(textGroup->layout())->addWidget(textBar);
    layout->addWidget(textGroup);

    QWidget *leaderGroup = createGroup("引线", tab);
    QToolBar *leaderBar = createToolBar(leaderGroup);
    leaderBar->addAction("多重引线");
    leaderBar->addAction("公差");
    static_cast<QVBoxLayout*>(leaderGroup->layout())->addWidget(leaderBar);
    layout->addWidget(leaderGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createToolsTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *blockGroup = createGroup("块", tab);
    QToolBar *blockBar = createToolBar(blockGroup);
    m_actionBlock = blockBar->addAction("创建块");
    blockBar->addAction("插入块");
    blockBar->addAction("块编辑器");
    blockBar->addAction("块管理器");
    static_cast<QVBoxLayout*>(blockGroup->layout())->addWidget(blockBar);
    layout->addWidget(blockGroup);

    QWidget *queryGroup = createGroup("查询", tab);
    QToolBar *queryBar = createToolBar(queryGroup);
    queryBar->addAction("距离");
    queryBar->addAction("面积");
    queryBar->addAction("点坐标");
    static_cast<QVBoxLayout*>(queryGroup->layout())->addWidget(queryBar);
    layout->addWidget(queryGroup);

    QWidget *aiGroup = createGroup("AI工具", tab);
    QToolBar *aiBar = createToolBar(aiGroup);
    aiBar->addAction("图纸精简");
    aiBar->addAction("自动布放");
    aiBar->addAction("材料估算");
    aiBar->addAction("自动检验");
    static_cast<QVBoxLayout*>(aiGroup->layout())->addWidget(aiBar);
    layout->addWidget(aiGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createViewTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *navGroup = createGroup("导航", tab);
    QToolBar *navBar = createToolBar(navGroup);
    navBar->addAction("平移");
    navBar->addAction("缩放");
    navBar->addAction("全部显示");
    navBar->addAction("窗口缩放");
    static_cast<QVBoxLayout*>(navGroup->layout())->addWidget(navBar);
    layout->addWidget(navGroup);

    QWidget *snapGroup = createGroup("捕捉", tab);
    QToolBar *snapBar = createToolBar(snapGroup);
    snapBar->addAction("对象捕捉");
    snapBar->addAction("正交模式");
    snapBar->addAction("极轴追踪");
    snapBar->addAction("栅格显示");
    static_cast<QVBoxLayout*>(snapGroup->layout())->addWidget(snapBar);
    layout->addWidget(snapGroup);

    QWidget *layerGroup = createGroup("图层", tab);
    QToolBar *layerBar = createToolBar(layerGroup);
    layerBar->addAction("图层特性");
    layerBar->addAction("图层状态");
    static_cast<QVBoxLayout*>(layerGroup->layout())->addWidget(layerBar);
    layout->addWidget(layerGroup);

    layout->addStretch();
    return tab;
}

QWidget *RibbonMenu::createOutputTab()
{
    QWidget *tab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(tab);
    layout->setSpacing(8);
    layout->setContentsMargins(8, 4, 8, 4);

    QWidget *printGroup = createGroup("打印", tab);
    QToolBar *printBar = createToolBar(printGroup);
    m_actionPrint = printBar->addAction("打印");
    printBar->addAction("批量打印");
    printBar->addAction("打印预览");
    printBar->addAction("页面设置");
    static_cast<QVBoxLayout*>(printGroup->layout())->addWidget(printBar);
    layout->addWidget(printGroup);

    QWidget *exportGroup = createGroup("导出", tab);
    QToolBar *exportBar = createToolBar(exportGroup);
    m_actionExport = exportBar->addAction("导出PDF");
    exportBar->addAction("导出DXF");
    exportBar->addAction("导出DWG");
    exportBar->addAction("导出Excel");
    static_cast<QVBoxLayout*>(exportGroup->layout())->addWidget(exportBar);
    layout->addWidget(exportGroup);

    QWidget *titleGroup = createGroup("图签", tab);
    QToolBar *titleBar = createToolBar(titleGroup);
    titleBar->addAction("插入图签");
    titleBar->addAction("图例");
    titleBar->addAction("材料表");
    static_cast<QVBoxLayout*>(titleGroup->layout())->addWidget(titleBar);
    layout->addWidget(titleGroup);

    layout->addStretch();
    return tab;
}

void RibbonMenu::setupActions(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    if (!mainWindow) return;

    // 绘图工具
    connect(m_actionLine, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("line"); });
    connect(m_actionCircle, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("circle"); });
    connect(m_actionRect, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("rectangle"); });
    connect(m_actionArc, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("arc"); });
    connect(m_actionText, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("text"); });
    connect(m_actionFeeder, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("feeder"); });

    // 编辑工具
    connect(m_actionMove, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("move"); });
    connect(m_actionCopy, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("copy"); });
    connect(m_actionRotate, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("rotate"); });
    connect(m_actionScale, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("scale"); });
    connect(m_actionMirror, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("mirror"); });
    connect(m_actionDelete, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onErase(); });
    connect(m_actionUndo, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onUndo(); });
    connect(m_actionRedo, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onRedo(); });

    // 室分器件
    connect(m_actionAntenna, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->placeDevice(Zhifen::DeviceItem::OmniAntenna, "全向吸顶天线"); });
    connect(m_actionCoupler, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->placeDevice(Zhifen::DeviceItem::Coupler, "耦合器"); });
    connect(m_actionSplitter, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->placeDevice(Zhifen::DeviceItem::Splitter, "功分器"); });
    connect(m_actionSource, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->placeDevice(Zhifen::DeviceItem::MacroBS, "宏基站"); });

    // 标注
    connect(m_actionDimLinear, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("dim_linear"); });
    connect(m_actionDimAligned, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("dim_aligned"); });
    connect(m_actionDimRadius, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->setCurrentTool("dim_radius"); });

    // 其他
    connect(m_actionLayer, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onLayerManager(); });
    connect(m_actionPrint, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onPrint(); });
    connect(m_actionExport, &QAction::triggered, mainWindow, [mainWindow](){ mainWindow->onExportDxf(); });
}

} // namespace Zhifen
