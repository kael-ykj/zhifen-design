#include "blockcreatedialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>

namespace Zhifen {

BlockCreateDialog::BlockCreateDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void BlockCreateDialog::setupUI()
{
    setWindowTitle("创建块");
    setMinimumWidth(350);
    setStyleSheet("QDialog { background: #252526; } QLabel { color: #ccc; } QLineEdit { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 4px; } QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 16px; } QPushButton:hover { background: #1177bb; } QCheckBox { color: #ccc; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 名称
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(8);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("输入块名称");
    formLayout->addRow("块名称:", m_nameEdit);

    // 基点
    QHBoxLayout *baseLayout = new QHBoxLayout();
    m_basePointLabel = new QLabel("基点: (0.00, 0.00)", this);
    m_basePointLabel->setStyleSheet("color: #4ec9b0;");
    m_pickBtn = new QPushButton("拾取点", this);
    m_pickBtn->setFixedWidth(80);
    connect(m_pickBtn, &QPushButton::clicked, this, &BlockCreateDialog::onPickPoint);
    baseLayout->addWidget(m_basePointLabel);
    baseLayout->addStretch();
    baseLayout->addWidget(m_pickBtn);
    formLayout->addRow("基点:", baseLayout);

    mainLayout->addLayout(formLayout);

    // 选项
    QGroupBox *optionsGroup = new QGroupBox("选项", this);
    optionsGroup->setStyleSheet("QGroupBox { color: #ccc; border: 1px solid #555; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);
    m_convertCheck = new QCheckBox("将对象转换为块", this);
    m_convertCheck->setChecked(true);
    m_deleteCheck = new QCheckBox("删除原对象", this);
    optionsLayout->addWidget(m_convertCheck);
    optionsLayout->addWidget(m_deleteCheck);
    mainLayout->addWidget(optionsGroup);

    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->setStyleSheet("QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 16px; } QPushButton:hover { background: #1177bb; }");
    m_okBtn = buttonBox->button(QDialogButtonBox::Ok);
    m_cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &BlockCreateDialog::onOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &BlockCreateDialog::onCancel);
    mainLayout->addWidget(buttonBox);

    m_basePoint = QPointF(0, 0);
}

void BlockCreateDialog::onPickPoint()
{
    hide();
    emit pickPointRequested();
}

void BlockCreateDialog::onOk()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_nameEdit->setFocus();
        return;
    }
    accept();
}

void BlockCreateDialog::onCancel()
{
    reject();
}

} // namespace Zhifen
