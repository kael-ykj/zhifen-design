#include "devicepanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QGroupBox>

DevicePanel::DevicePanel(QWidget *parent)
    : QDockWidget("器件库", parent)
{
    setupUI();
    loadDevices();
}

void DevicePanel::setupUI()
{
    QWidget *container = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(container);

    m_list = new QListWidget(this);
    m_list->setStyleSheet("QListWidget { background: #252526; color: #ccc; border: 1px solid #3c3c3c; } QListWidget::item:selected { background: #0e639c; }");
    connect(m_list, &QListWidget::itemClicked, this, &DevicePanel::onItemClicked);
    layout->addWidget(m_list);

    m_placeBtn = new QPushButton("放置器件", this);
    m_placeBtn->setStyleSheet("QPushButton { background: #0e639c; color: white; padding: 6px; border: none; border-radius: 3px; } QPushButton:hover { background: #1177bb; }");
    connect(m_placeBtn, &QPushButton::clicked, this, &DevicePanel::onPlaceClicked);
    layout->addWidget(m_placeBtn);

    setWidget(container);
}

void DevicePanel::loadDevices()
{
    QStringList categories = {"天线", "功分器", "耦合器", "合路器", "信源"};
    QMap<QString, QList<QPair<QString, DeviceType>>> devices;
    devices["天线"] = {{"全向吸顶天线", DevAntennaOmni}, {"定向壁挂天线", DevAntennaDirectional}, {"对数周期天线", DevAntennaLPDA}};
    devices["功分器"] = {{"二功分器", DevSplitter2}, {"三功分器", DevSplitter3}, {"四功分器", DevSplitter4}};
    devices["耦合器"] = {{"5dB耦合器", DevCoupler5}, {"7dB耦合器", DevCoupler7}, {"10dB耦合器", DevCoupler10}, {"15dB耦合器", DevCoupler15}, {"20dB耦合器", DevCoupler20}};
    devices["合路器"] = {{"合路器", DevCombiner}};
    devices["信源"] = {{"RRU", DevSourceRRU}, {"BBU", DevSourceBBU}, {"微基站", DevSourceMicro}, {"直放站", DevSourceRepeater}};

    for (const QString &cat : categories) {
        QListWidgetItem *header = new QListWidgetItem("【" + cat + "】");
        header->setFlags(header->flags() & ~Qt::ItemIsSelectable);
        header->setForeground(QColor("#888"));
        m_list->addItem(header);
        for (const auto &dev : devices[cat]) {
            QListWidgetItem *item = new QListWidgetItem("  " + dev.first);
            item->setData(Qt::UserRole, dev.second);
            m_list->addItem(item);
        }
    }
}

void DevicePanel::onItemClicked(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    m_currentType = static_cast<DeviceType>(item->data(Qt::UserRole).toInt());
    emit deviceSelected(m_currentType);
}

void DevicePanel::onPlaceClicked()
{
    emit placeDevice(m_currentType);
}
