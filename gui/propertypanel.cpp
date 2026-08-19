#include "propertypanel.h"
#include <QVBoxLayout>

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

    m_xEdit = new QLineEdit(this);
    form->addRow("X坐标:", m_xEdit);

    m_yEdit = new QLineEdit(this);
    form->addRow("Y坐标:", m_yEdit);

    m_noteEdit = new QLineEdit(this);
    form->addRow("备注:", m_noteEdit);

    mainLayout->addLayout(form);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_applyBtn = new QPushButton("应用", this);
    m_deleteBtn = new QPushButton("删除", this);
    m_deleteBtn->setStyleSheet("color: red;");
    btnLayout->addWidget(m_applyBtn);
    btnLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(btnLayout);

    mainLayout->addStretch();

    clear();
}

void PropertyPanel::setDeviceId(const QString& deviceId)
{
    m_currentDeviceId = deviceId;
    m_titleLabel->setText("器件属性");
    m_idEdit->setText(deviceId);
    m_modelEdit->setText("");
    m_xEdit->setText("");
    m_yEdit->setText("");
    m_noteEdit->setText("");
    m_applyBtn->setEnabled(true);
    m_deleteBtn->setEnabled(true);
}

void PropertyPanel::clear()
{
    m_currentDeviceId.clear();
    m_titleLabel->setText("未选中器件");
    m_idEdit->clear();
    m_modelEdit->clear();
    m_xEdit->clear();
    m_yEdit->clear();
    m_noteEdit->clear();
    m_applyBtn->setEnabled(false);
    m_deleteBtn->setEnabled(false);
}
