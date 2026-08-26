#include "propertypanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <algorithm>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    m_titleLabel = new QLabel("未选中器件", this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    mainLayout->addWidget(m_titleLabel);

    QFormLayout* form = new QFormLayout();
    m_idEdit = new QLineEdit(this);
    m_idEdit->setReadOnly(true);
    form->addRow("器件ID:", m_idEdit);

    m_modelEdit = new QLineEdit(this);
    m_modelEdit->setReadOnly(true);
    form->addRow("型号:", m_modelEdit);

    m_categoryEdit = new QLineEdit(this);
    m_categoryEdit->setReadOnly(true);
    form->addRow("类别:", m_categoryEdit);

    m_xEdit = new QLineEdit(this);
    form->addRow("X坐标:", m_xEdit);

    m_yEdit = new QLineEdit(this);
    form->addRow("Y坐标:", m_yEdit);

    m_noteEdit = new QLineEdit(this);
    form->addRow("备注:", m_noteEdit);

    mainLayout->addLayout(form);

    m_powerLabel = new QLabel("", this);
    m_powerLabel->setStyleSheet("color: green; font-size: 10px;");
    m_powerLabel->setWordWrap(true);
    mainLayout->addWidget(m_powerLabel);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_applyBtn = new QPushButton("应用修改", this);
    m_deleteBtn = new QPushButton("删除器件", this);
    m_deleteBtn->setStyleSheet("color: red;");
    btnLayout->addWidget(m_applyBtn);
    btnLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_applyBtn, &QPushButton::clicked, this, &PropertyPanel::onApply);
    connect(m_deleteBtn, &QPushButton::clicked, this, &PropertyPanel::onDelete);

    mainLayout->addStretch();
    clear();
}

void PropertyPanel::setProject(zf::Project* project)
{
    m_project = project;
}

static QString categoryToString(zf::DeviceCategory cat) {
    switch (cat) {
        case zf::DeviceCategory::SIGNAL_SOURCE: return "信源";
        case zf::DeviceCategory::SPLITTER: return "功分器";
        case zf::DeviceCategory::COUPLER: return "耦合器";
        case zf::DeviceCategory::COMBINER: return "合路器";
        case zf::DeviceCategory::ANTENNA: return "天线";
        case zf::DeviceCategory::CABLE: return "线缆";
        case zf::DeviceCategory::LOAD: return "负载";
        default: return "其他";
    }
}

void PropertyPanel::setDeviceId(const QString& deviceId)
{
    m_currentDeviceId = deviceId;
    refresh();
}

void PropertyPanel::refresh()
{
    if (m_currentDeviceId.isEmpty() || !m_project || m_project->floors.empty()) {
        clear();
        return;
    }

    const auto& floor = m_project->floors[0];
    auto it = std::find_if(floor.devices.begin(), floor.devices.end(),
        [&](const zf::DeviceInstance& d) { return d.instanceId == m_currentDeviceId.toStdString(); });

    if (it == floor.devices.end()) {
        clear();
        return;
    }

    const auto& dev = *it;
    m_titleLabel->setText("器件属性");
    m_idEdit->setText(QString::fromStdString(dev.instanceId));
    m_modelEdit->setText(QString::fromStdString(dev.modelId));
    m_xEdit->setText(QString::number(dev.position.x, 'f', 1));
    m_yEdit->setText(QString::number(dev.position.y, 'f', 1));
    m_noteEdit->setText(QString::fromStdString(dev.notes));

    // 查找型号信息
    auto mit = std::find_if(m_project->deviceLibrary.begin(), m_project->deviceLibrary.end(),
        [&](const zf::DeviceModel& m) { return m.modelId == dev.modelId; });
    if (mit != m_project->deviceLibrary.end()) {
        m_categoryEdit->setText(categoryToString(mit->category));
    } else {
        m_categoryEdit->setText("未知");
    }

    // 功率信息
    if (!dev.outputPower_dBm.empty()) {
        QString powerText = "输出功率:\n";
        for (const auto& p : dev.outputPower_dBm) {
            powerText += QString("  端口%1: %2 dBm\n").arg(QString::fromStdString(p.first)).arg(p.second, 0, 'f', 1);
        }
        m_powerLabel->setText(powerText);
    } else {
        m_powerLabel->setText("(未计算功率，运行链路预算后显示)");
    }

    m_applyBtn->setEnabled(true);
    m_deleteBtn->setEnabled(true);
}

void PropertyPanel::clear()
{
    m_currentDeviceId.clear();
    m_titleLabel->setText("未选中器件");
    m_idEdit->clear();
    m_modelEdit->clear();
    m_categoryEdit->clear();
    m_xEdit->clear();
    m_yEdit->clear();
    m_noteEdit->clear();
    m_powerLabel->clear();
    m_applyBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
}

void PropertyPanel::onApply()
{
    if (!m_project || m_project->floors.empty() || m_currentDeviceId.isEmpty()) return;

    bool okX, okY;
    double x = m_xEdit->text().toDouble(&okX);
    double y = m_yEdit->text().toDouble(&okY);
    if (!okX || !okY) {
        QMessageBox::warning(this, "输入错误", "坐标必须是数字");
        return;
    }

    auto& floor = m_project->floors[0];
    for (auto& dev : floor.devices) {
        if (dev.instanceId == m_currentDeviceId.toStdString()) {
            dev.position.x = x;
            dev.position.y = y;
            dev.notes = m_noteEdit->text().toStdString();
            break;
        }
    }
    emit propertyChanged();
}

void PropertyPanel::onDelete()
{
    if (!m_project || m_project->floors.empty() || m_currentDeviceId.isEmpty()) return;

    if (QMessageBox::question(this, "确认删除",
        QString("确定要删除器件 %1 吗？").arg(m_currentDeviceId)) != QMessageBox::Yes) {
        return;
    }

    QString deletedId = m_currentDeviceId;
    auto& floor = m_project->floors[0];
    floor.devices.erase(std::remove_if(floor.devices.begin(), floor.devices.end(),
        [&](const zf::DeviceInstance& d) { return d.instanceId == deletedId.toStdString(); }),
        floor.devices.end());

    // 同时清理连接
    for (auto& dev : floor.devices) {
        dev.connections.erase(std::remove_if(dev.connections.begin(), dev.connections.end(),
            [&](const zf::Connection& c) { return c.targetInstanceId == deletedId.toStdString(); }),
            dev.connections.end());
    }

    clear();
    emit deviceDeleted(deletedId);
}
