#include "devicelistpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

DeviceListPanel::DeviceListPanel(zf::DeviceLibrary* devLib, QWidget *parent)
    : QWidget(parent), m_devLib(devLib)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    // 类别选择
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("全部", -1);
    m_categoryCombo->addItem("信源", (int)zf::DeviceCategory::SIGNAL_SOURCE);
    m_categoryCombo->addItem("功分器", (int)zf::DeviceCategory::SPLITTER);
    m_categoryCombo->addItem("耦合器", (int)zf::DeviceCategory::COUPLER);
    m_categoryCombo->addItem("合路器", (int)zf::DeviceCategory::COMBINER);
    m_categoryCombo->addItem("天线", (int)zf::DeviceCategory::ANTENNA);
    m_categoryCombo->addItem("线缆", (int)zf::DeviceCategory::CABLE);
    m_categoryCombo->addItem("负载", (int)zf::DeviceCategory::LOAD);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DeviceListPanel::onCategoryChanged);
    layout->addWidget(m_categoryCombo);

    // 器件列表
    m_deviceList = new QListWidget(this);
    m_deviceList->setViewMode(QListView::ListMode);
    m_deviceList->setSpacing(2);
    connect(m_deviceList, &QListWidget::itemClicked,
            this, &DeviceListPanel::onDeviceClicked);
    layout->addWidget(m_deviceList, 1);

    // 信息标签
    m_infoLabel = new QLabel("点击器件放置到画布", this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: gray; font-size: 10px;");
    layout->addWidget(m_infoLabel);

    refreshList();
}

void DeviceListPanel::onCategoryChanged(int index)
{
    Q_UNUSED(index);
    refreshList();
}

void DeviceListPanel::onDeviceClicked(QListWidgetItem* item)
{
    QString modelId = item->data(Qt::UserRole).toString();
    emit deviceModelSelected(modelId);

    auto model = m_devLib->getModelById(modelId.toStdString());
    if (model) {
        QString info = QString("%1\n增益: %2 dBi\n端口: %3个")
            .arg(QString::fromStdString(model->displayName))
            .arg(model->gain_dBi)
            .arg(model->portCount);
        m_infoLabel->setText(info);
    }
}

void DeviceListPanel::refreshList()
{
    m_deviceList->clear();
    int cat = m_categoryCombo->currentData().toInt();

    std::vector<zf::DeviceModel> models;
    if (cat == -1) {
        for (const auto& c : {zf::DeviceCategory::SIGNAL_SOURCE, zf::DeviceCategory::SPLITTER,
                              zf::DeviceCategory::COUPLER, zf::DeviceCategory::COMBINER,
                              zf::DeviceCategory::ANTENNA, zf::DeviceCategory::CABLE,
                              zf::DeviceCategory::LOAD}) {
            auto list = m_devLib->getModelsByCategory(c);
            models.insert(models.end(), list.begin(), list.end());
        }
    } else {
        models = m_devLib->getModelsByCategory((zf::DeviceCategory)cat);
    }

    for (const auto& m : models) {
        QListWidgetItem* item = new QListWidgetItem(
            QString::fromStdString(m.displayName), m_deviceList);
        item->setData(Qt::UserRole, QString::fromStdString(m.modelId));
        item->setToolTip(QString("型号: %1\n增益: %2 dBi")
            .arg(QString::fromStdString(m.modelId))
            .arg(m.gain_dBi));
    }
}
