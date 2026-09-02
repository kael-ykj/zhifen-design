#include "cadstatusbar.h"
#include <QHBoxLayout>
#include <QWidget>
#include <QPointF>

namespace Zhifen {

CadStatusBar::CadStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    setStyleSheet(R"(
        QStatusBar { background: #3c3f41; color: #ccc; border-top: 1px solid #222; }
        QStatusBar::item { border: none; }
        QLabel { color: #ccc; padding: 0 8px; font-size: 11px; }
        QPushButton { background: transparent; color: #999; border: none; padding: 4px 10px; font-size: 11px; }
        QPushButton:hover { background: #505355; color: #fff; }
        QPushButton:checked { background: #0e639c; color: #fff; font-weight: bold; }
    )");

    // 左侧：坐标
    m_coordLabel = new QLabel("X: 0.00  Y: 0.00", this);
    m_coordLabel->setMinimumWidth(150);
    addWidget(m_coordLabel);

    // 中间：命令提示
    m_commandLabel = new QLabel("就绪", this);
    m_commandLabel->setMinimumWidth(200);
    addWidget(m_commandLabel);

    // 选择计数
    m_selectionLabel = new QLabel("", this);
    addWidget(m_selectionLabel);

    addPermanentWidget(new QLabel(" ", this));

    // 右侧：标准CAD模式切换按钮（按AutoCAD顺序）
    m_snapBtn = createToggleButton("捕捉");
    m_gridBtn = createToggleButton("栅格");
    m_orthoBtn = createToggleButton("正交");
    m_polarBtn = createToggleButton("极轴");
    m_osnapBtn = createToggleButton("对象捕捉");
    m_dynBtn = createToggleButton("动态输入");
    m_lwtBtn = createToggleButton("线宽");
    m_modelBtn = createToggleButton("模型空间");
    m_workspaceBtn = createToggleButton("室分设计");

    m_snapBtn->setChecked(true);
    m_osnapBtn->setChecked(true);
    m_modelBtn->setChecked(true);
    m_workspaceBtn->setChecked(true);

    addPermanentWidget(m_snapBtn);
    addPermanentWidget(m_gridBtn);
    addPermanentWidget(m_orthoBtn);
    addPermanentWidget(m_polarBtn);
    addPermanentWidget(m_osnapBtn);
    addPermanentWidget(m_dynBtn);
    addPermanentWidget(m_lwtBtn);
    addPermanentWidget(m_modelBtn);
    addPermanentWidget(m_workspaceBtn);

    connect(m_snapBtn, &QPushButton::toggled, this, &CadStatusBar::snapToggled);
    connect(m_orthoBtn, &QPushButton::toggled, this, &CadStatusBar::orthoToggled);
    connect(m_gridBtn, &QPushButton::toggled, this, &CadStatusBar::gridToggled);
    connect(m_polarBtn, &QPushButton::toggled, this, &CadStatusBar::polarToggled);
    connect(m_osnapBtn, &QPushButton::toggled, this, &CadStatusBar::osnapToggled);
    connect(m_dynBtn, &QPushButton::toggled, this, &CadStatusBar::dynToggled);
}

QPushButton *CadStatusBar::createToggleButton(const QString &text)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void CadStatusBar::setCoordinate(const QPointF &pos)
{
    m_coordLabel->setText(QString("X: %1  Y: %2").arg(pos.x(), 0, 'f', 2).arg(pos.y(), 0, 'f', 2));
}

void CadStatusBar::setCommand(const QString &cmd)
{
    m_commandLabel->setText(cmd);
}

void CadStatusBar::setSelectionCount(int count)
{
    if (count > 0) {
        m_selectionLabel->setText(QString("已选中 %1 个对象").arg(count));
    } else {
        m_selectionLabel->setText("");
    }
}

} // namespace Zhifen
