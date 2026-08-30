#include "attributedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QDialogButtonBox>

namespace Zhifen {

AttributeDialog::AttributeDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

void AttributeDialog::setupUI()
{
    setWindowTitle("编辑属性");
    setMinimumWidth(400);
    setMinimumHeight(300);
    setStyleSheet("QDialog { background: #252526; } QLabel { color: #ccc; } QLineEdit { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 4px; } QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 16px; } QPushButton:hover { background: #1177bb; } QGroupBox { color: #ccc; border: 1px solid #555; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 标题
    QLabel *titleLabel = new QLabel("请输入块的属性值", this);
    titleLabel->setStyleSheet("color: #fff; font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    // 属性滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    QWidget *scrollWidget = new QWidget();
    m_attrLayout = new QVBoxLayout(scrollWidget);
    m_attrLayout->setSpacing(8);
    m_attrLayout->setContentsMargins(4, 4, 4, 4);
    m_attrLayout->addStretch();

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea, 1);

    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okBtn = buttonBox->button(QDialogButtonBox::Ok);
    m_cancelBtn = buttonBox->button(QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AttributeDialog::onOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AttributeDialog::onCancel);
    mainLayout->addWidget(buttonBox);
}

void AttributeDialog::setBlock(BlockDefinition *def, const QMap<QString, QString> &currentValues)
{
    clearAttributes();
    m_values = currentValues;

    if (!def) return;

    setWindowTitle(QString("编辑属性 - %1").arg(def->name()));

    for (const auto &attr : def->attributes()) {
        QGroupBox *attrGroup = new QGroupBox(attr.tag, this);
        attrGroup->setStyleSheet("QGroupBox { color: #4ec9b0; border: 1px solid #555; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");

        QVBoxLayout *attrLayout = new QVBoxLayout(attrGroup);

        // 提示
        if (!attr.prompt.isEmpty()) {
            QLabel *promptLabel = new QLabel(attr.prompt, attrGroup);
            promptLabel->setStyleSheet("color: #999; font-size: 11px;");
            attrLayout->addWidget(promptLabel);
        }

        // 输入框
        QLineEdit *edit = new QLineEdit(attrGroup);
        QString value = currentValues.value(attr.tag, attr.defaultValue);
        edit->setText(value);
        edit->setPlaceholderText(attr.defaultValue);
        if (attr.constant) {
            edit->setEnabled(false);
            edit->setStyleSheet("QLineEdit { background: #2a2a2a; color: #888; border: 1px solid #444; padding: 4px; }");
        }
        attrLayout->addWidget(edit);

        m_edits[attr.tag] = edit;
        m_attrLayout->insertWidget(m_attrLayout->count() - 1, attrGroup);
    }
}

void AttributeDialog::clearAttributes()
{
    QLayoutItem *item;
    while ((item = m_attrLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_attrLayout->addStretch();
    m_edits.clear();
}

void AttributeDialog::onOk()
{
    for (auto it = m_edits.begin(); it != m_edits.end(); ++it) {
        m_values[it.key()] = it.value()->text();
    }
    accept();
}

void AttributeDialog::onCancel()
{
    reject();
}

} // namespace Zhifen
