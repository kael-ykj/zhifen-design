#include "devicelibrarypanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QHeaderView>
#include <QTreeWidgetItem>

namespace Zhifen {

DeviceLibraryPanel::DeviceLibraryPanel(QWidget *parent)
    : QDockWidget("器件库", parent), m_selectedType(DeviceItem::OmniAntenna)
{
    setupUI();
    buildTree();
}

void DeviceLibraryPanel::setupUI()
{
    setStyleSheet(R"(
        QDockWidget { background: #252526; color: #ccc; }
        QDockWidget::title { background: #2d2d30; padding: 6px; }
        QTreeWidget { background: #1e1e1e; color: #ccc; border: 1px solid #3c3c3c; }
        QTreeWidget::item:selected { background: #0e639c; }
        QTreeWidget::item:hover { background: #2a2d2e; }
        QLineEdit { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 4px; }
        QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 12px; }
        QPushButton:hover { background: #1177bb; }
        QLabel { color: #999; }
    )");

    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setSpacing(6);
    layout->setContentsMargins(8, 8, 8, 8);

    // 搜索框
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索器件...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DeviceLibraryPanel::onSearch);
    layout->addWidget(m_searchEdit);

    // 器件树
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({"器件名称", "类型"});
    m_treeWidget->setColumnWidth(0, 150);
    m_treeWidget->setColumnWidth(1, 80);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setRootIsDecorated(true);
    connect(m_treeWidget, &QTreeWidget::itemClicked, this, &DeviceLibraryPanel::onItemClicked);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int col) {
        onItemClicked(item, col);
        onPlaceClicked();
    });
    layout->addWidget(m_treeWidget, 1);

    // 放置按钮
    m_placeBtn = new QPushButton("放置器件", this);
    m_placeBtn->setEnabled(false);
    connect(m_placeBtn, &QPushButton::clicked, this, &DeviceLibraryPanel::onPlaceClicked);
    layout->addWidget(m_placeBtn);

    setWidget(content);
    setMinimumWidth(220);
}

void DeviceLibraryPanel::buildTree()
{
    m_treeWidget->clear();

    // 1. 信号源类
    addDeviceCategory("信号源", {
        {"宏基站", DeviceItem::MacroBS},
        {"微基站", DeviceItem::MicroBS},
        {"光纤直放站", DeviceItem::FiberRepeater},
        {"BBU", DeviceItem::BBU},
        {"RRU", DeviceItem::RRU},
        {"皮基站pRRU", DeviceItem::PicoStation},
        {"光端机", DeviceItem::OpticalTerminal},
        {"干线放大器", DeviceItem::Repeater},
    });

    // 2. 天线类
    addDeviceCategory("天线", {
        {"全向吸顶天线", DeviceItem::OmniAntenna},
        {"定向壁挂天线", DeviceItem::DirectionalAntenna},
        {"射灯天线", DeviceItem::SpotlightAntenna},
        {"外引天线", DeviceItem::ExternalAntenna},
        {"吸顶天线", DeviceItem::CeilingAntenna},
        {"八木天线", DeviceItem::YagiAntenna},
        {"栅格天线", DeviceItem::GridAntenna},
        {"电梯天线", DeviceItem::ElevatorAntenna},
        {"GPS天线", DeviceItem::GPSAntenna},
        {"对数周期天线", DeviceItem::LogPeriodicAntenna},
    });

    // 3. 耦合器类
    addDeviceCategory("耦合器", {
        {"5dB耦合器", DeviceItem::Coupler},
        {"6dB耦合器", DeviceItem::Coupler},
        {"7dB耦合器", DeviceItem::Coupler},
        {"10dB耦合器", DeviceItem::Coupler},
        {"12dB耦合器", DeviceItem::Coupler},
        {"15dB耦合器", DeviceItem::Coupler},
        {"20dB耦合器", DeviceItem::Coupler},
        {"30dB耦合器", DeviceItem::Coupler},
        {"40dB耦合器", DeviceItem::Coupler},
    });

    // 4. 功分器类
    addDeviceCategory("功分器", {
        {"二功分器", DeviceItem::Splitter},
        {"三功分器", DeviceItem::Splitter},
        {"四功分器", DeviceItem::Splitter},
        {"腔体二功分", DeviceItem::CavitySplitter},
        {"腔体三功分", DeviceItem::CavitySplitter},
    });

    // 5. 无源器件类
    addDeviceCategory("无源器件", {
        {"合路器", DeviceItem::Combiner},
        {"3dB电桥", DeviceItem::Hybrid},
        {"终端负载", DeviceItem::Load},
        {"衰减器", DeviceItem::Attenuator},
        {"固定衰减器", DeviceItem::FixedAttenuator},
        {"可调衰减器", DeviceItem::VariableAttenuator},
        {"隔离器", DeviceItem::Isolator},
        {"环形器", DeviceItem::Circulator},
        {"滤波器", DeviceItem::Filter},
        {"POI合路平台", DeviceItem::POI},
    });

    // 6. 馈线类
    addDeviceCategory("馈线", {
        {"1/2馈线", DeviceItem::OmniAntenna}, // 馈线通过FeederTool绘制，这里仅作分类展示
        {"7/8馈线", DeviceItem::OmniAntenna},
        {"1-5/8馈线", DeviceItem::OmniAntenna},
        {"跳线", DeviceItem::OmniAntenna},
        {"漏缆", DeviceItem::OmniAntenna},
    });

    // 7. 数字化室分类
    addDeviceCategory("数字化室分", {
        {"MAU主控单元", DeviceItem::MAU},
        {"EU扩展单元", DeviceItem::EU},
        {"pRRU远端单元", DeviceItem::PRRU},
        {"POE交换机", DeviceItem::POESwitch},
        {"光模块", DeviceItem::OpticalModule},
    });

    // 8. 连接与辅件类
    addDeviceCategory("连接与辅件", {
        {"主接点", DeviceItem::MainJunction},
        {"副接点", DeviceItem::SubJunction},
        {"N型接头", DeviceItem::NConnector},
        {"DIN型接头", DeviceItem::DINConnector},
        {"接地卡", DeviceItem::GroundingKit},
        {"接地", DeviceItem::Ground},
        {"防雷器", DeviceItem::LightningProtector},
        {"浪涌保护器", DeviceItem::SurgeProtector},
        {"开关", DeviceItem::Switch},
    });

    m_treeWidget->expandAll();
}

void DeviceLibraryPanel::addDeviceCategory(const QString &category, const QList<QPair<QString, DeviceItem::DeviceType>> &devices)
{
    QTreeWidgetItem *catItem = new QTreeWidgetItem(m_treeWidget);
    catItem->setText(0, category);
    catItem->setBackground(0, QColor("#3c3c3c"));
    catItem->setForeground(0, QColor("#fff"));
    catItem->setFont(0, QFont("Microsoft YaHei", 10, QFont::Bold));

    for (const auto &dev : devices) {
        QTreeWidgetItem *item = new QTreeWidgetItem(catItem);
        item->setText(0, dev.first);
        item->setText(1, DeviceItem::deviceTypeName(dev.second));
        item->setData(0, Qt::UserRole, static_cast<int>(dev.second));
        item->setData(0, Qt::UserRole + 1, dev.first);
    }
}

void DeviceLibraryPanel::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item || item->childCount() > 0) {
        m_placeBtn->setEnabled(false);
        return;
    }

    bool ok;
    int type = item->data(0, Qt::UserRole).toInt(&ok);
    if (ok) {
        m_selectedType = static_cast<DeviceItem::DeviceType>(type);
        m_selectedName = item->data(0, Qt::UserRole + 1).toString();
        m_placeBtn->setEnabled(true);
    }
}

void DeviceLibraryPanel::onPlaceClicked()
{
    if (m_placeBtn->isEnabled()) {
        emit deviceSelected(m_selectedType, m_selectedName);
    }
}

void DeviceLibraryPanel::onSearch(const QString &text)
{
    if (text.isEmpty()) {
        for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
            m_treeWidget->topLevelItem(i)->setHidden(false);
            for (int j = 0; j < m_treeWidget->topLevelItem(i)->childCount(); j++) {
                m_treeWidget->topLevelItem(i)->child(j)->setHidden(false);
            }
        }
        return;
    }

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *cat = m_treeWidget->topLevelItem(i);
        bool hasVisibleChild = false;
        for (int j = 0; j < cat->childCount(); j++) {
            QTreeWidgetItem *child = cat->child(j);
            bool match = child->text(0).contains(text, Qt::CaseInsensitive) ||
                         child->text(1).contains(text, Qt::CaseInsensitive);
            child->setHidden(!match);
            if (match) hasVisibleChild = true;
        }
        cat->setHidden(!hasVisibleChild);
    }
}

void DeviceLibraryPanel::refresh()
{
    buildTree();
}

} // namespace Zhifen
