#include "toolbox.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QIcon>

namespace Zhifen {

ToolBox::ToolBox(QWidget *parent)
    : QDockWidget("工具箱", parent)
{
    setStyleSheet(R"(
        QDockWidget { background: #252526; color: #ccc; }
        QDockWidget::title { background: #2d2d30; padding: 6px; }
        QToolBox { background: #252526; border: none; }
        QToolBox::tab { background: #2d2d30; color: #ccc; padding: 8px; border: 1px solid #3c3c3c; border-bottom: none; }
        QToolBox::tab:selected { background: #0e639c; color: #fff; }
        QToolButton { background: #3c3c3c; color: #ccc; border: 1px solid #555; border-radius: 3px; padding: 8px; font-size: 11px; }
        QToolButton:hover { background: #0e639c; color: #fff; border-color: #0e639c; }
        QToolButton:pressed { background: #1177bb; }
    )");

    m_toolBox = new QToolBox(this);
    m_toolBox->addItem(createDrawPage(), "绘图");
    m_toolBox->addItem(createEditPage(), "编辑");
    m_toolBox->addItem(createIndoorPage(), "室分");
    m_toolBox->addItem(createDimensionPage(), "标注");

    setWidget(m_toolBox);
    setMinimumWidth(140);
}

QToolButton *ToolBox::createToolButton(const QString &text, const QString &toolName)
{
    QToolButton *btn = new QToolButton(this);
    btn->setText(text);
    btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btn->setMinimumHeight(32);
    connect(btn, &QToolButton::clicked, this, [this, toolName]() {
        emit toolSelected(toolName);
    });
    return btn;
}

QWidget *ToolBox::createDrawPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(createToolButton("直线", "line"));
    layout->addWidget(createToolButton("圆", "circle"));
    layout->addWidget(createToolButton("圆弧", "arc"));
    layout->addWidget(createToolButton("矩形", "rect"));
    layout->addWidget(createToolButton("多段线", "polyline"));
    layout->addWidget(createToolButton("文字", "text"));
    layout->addWidget(createToolButton("图案填充", "hatch"));
    layout->addStretch();

    return page;
}

QWidget *ToolBox::createEditPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(createToolButton("移动", "move"));
    layout->addWidget(createToolButton("复制", "copy"));
    layout->addWidget(createToolButton("旋转", "rotate"));
    layout->addWidget(createToolButton("缩放", "scale"));
    layout->addWidget(createToolButton("镜像", "mirror"));
    layout->addWidget(createToolButton("偏移", "offset"));
    layout->addWidget(createToolButton("修剪", "trim"));
    layout->addWidget(createToolButton("延伸", "extend"));
    layout->addWidget(createToolButton("删除", "delete"));
    layout->addStretch();

    return page;
}

QWidget *ToolBox::createIndoorPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(createToolButton("信源", "source"));
    layout->addWidget(createToolButton("全向天线", "antenna_omni"));
    layout->addWidget(createToolButton("定向天线", "antenna_dir"));
    layout->addWidget(createToolButton("耦合器", "coupler"));
    layout->addWidget(createToolButton("功分器", "splitter"));
    layout->addWidget(createToolButton("合路器", "combiner"));
    layout->addWidget(createToolButton("馈线", "feeder"));
    layout->addWidget(createToolButton("漏缆", "leaky"));
    layout->addWidget(createToolButton("链路预算", "linkcalc"));
    layout->addStretch();

    return page;
}

QWidget *ToolBox::createDimensionPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setSpacing(4);
    layout->setContentsMargins(4, 4, 4, 4);

    layout->addWidget(createToolButton("线性标注", "dim_linear"));
    layout->addWidget(createToolButton("对齐标注", "dim_aligned"));
    layout->addWidget(createToolButton("半径标注", "dim_radius"));
    layout->addWidget(createToolButton("直径标注", "dim_diameter"));
    layout->addWidget(createToolButton("角度标注", "dim_angle"));
    layout->addWidget(createToolButton("引线标注", "leader"));
    layout->addStretch();

    return page;
}

} // namespace Zhifen
