#include "blockeditor.h"
#include "cad/cadscene.h"
#include "cad/cadview.h"
#include "blocks/blockmanager.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/textitem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QGraphicsItem>

namespace Zhifen {

BlockEditor::BlockEditor(const QString &blockName, QWidget *parent)
    : QDialog(parent), m_blockName(blockName)
{
    setWindowTitle(QString("块编辑器 - %1").arg(blockName));
    resize(900, 600);
    setStyleSheet("QDialog { background: #252526; } QLabel { color: #ccc; } QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 12px; } QPushButton:hover { background: #1177bb; } QPushButton:disabled { background: #3c3c3c; color: #888; } QListWidget { background: #1e1e1e; color: #ccc; border: 1px solid #3c3c3c; } QListWidget::item:selected { background: #0e639c; } QGroupBox { color: #ccc; border: 1px solid #555; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");

    m_scene = new CadScene(this);
    m_view = new CadView(this);
    m_view->setCadScene(m_scene);

    setupUI();
    loadBlock();
}

BlockEditor::~BlockEditor()
{
}

void BlockEditor::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // 工具栏
    QHBoxLayout *toolLayout = new QHBoxLayout();
    toolLayout->setSpacing(4);

    m_lineBtn = new QPushButton("直线", this);
    m_circleBtn = new QPushButton("圆", this);
    m_textBtn = new QPushButton("文字", this);
    m_deleteBtn = new QPushButton("删除", this);
    m_zoomBtn = new QPushButton("全部显示", this);

    connect(m_lineBtn, &QPushButton::clicked, this, &BlockEditor::onAddLine);
    connect(m_circleBtn, &QPushButton::clicked, this, &BlockEditor::onAddCircle);
    connect(m_textBtn, &QPushButton::clicked, this, &BlockEditor::onAddText);
    connect(m_deleteBtn, &QPushButton::clicked, this, &BlockEditor::onDeleteSelected);
    connect(m_zoomBtn, &QPushButton::clicked, this, &BlockEditor::onZoomExtents);

    toolLayout->addWidget(m_lineBtn);
    toolLayout->addWidget(m_circleBtn);
    toolLayout->addWidget(m_textBtn);
    toolLayout->addWidget(m_deleteBtn);
    toolLayout->addWidget(m_zoomBtn);
    toolLayout->addStretch();

    QLabel *titleLabel = new QLabel(QString("编辑块: %1").arg(m_blockName), this);
    titleLabel->setStyleSheet("color: #fff; font-size: 14px; font-weight: bold;");
    toolLayout->addWidget(titleLabel);

    mainLayout->addLayout(toolLayout);

    // 主分割区
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧：绘图区
    QWidget *drawWidget = new QWidget(this);
    QVBoxLayout *drawLayout = new QVBoxLayout(drawWidget);
    drawLayout->setContentsMargins(0, 0, 0, 0);
    drawLayout->addWidget(m_view);
    splitter->addWidget(drawWidget);

    // 右侧：属性编辑
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    QGroupBox *attrGroup = new QGroupBox("属性定义", this);
    QVBoxLayout *attrLayout = new QVBoxLayout(attrGroup);
    attrLayout->setSpacing(4);

    m_attrList = new QListWidget(this);
    attrLayout->addWidget(m_attrList);

    QHBoxLayout *attrBtnLayout = new QHBoxLayout();
    m_addAttrBtn = new QPushButton("添加", this);
    m_editAttrBtn = new QPushButton("编辑", this);
    m_delAttrBtn = new QPushButton("删除", this);
    connect(m_addAttrBtn, &QPushButton::clicked, this, &BlockEditor::onAddAttribute);
    connect(m_editAttrBtn, &QPushButton::clicked, this, &BlockEditor::onEditAttribute);
    connect(m_delAttrBtn, &QPushButton::clicked, this, &BlockEditor::onDeleteAttribute);
    attrBtnLayout->addWidget(m_addAttrBtn);
    attrBtnLayout->addWidget(m_editAttrBtn);
    attrBtnLayout->addWidget(m_delAttrBtn);
    attrLayout->addLayout(attrBtnLayout);

    rightLayout->addWidget(attrGroup);
    rightLayout->addStretch();

    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);

    // 底部按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton("保存", this);
    m_cancelBtn = new QPushButton("取消", this);
    m_saveBtn->setStyleSheet("background: #0e639c; color: #fff; border: none; padding: 8px 24px; font-size: 13px;");
    m_cancelBtn->setStyleSheet("background: #555; color: #fff; border: none; padding: 8px 24px; font-size: 13px;");
    connect(m_saveBtn, &QPushButton::clicked, this, &BlockEditor::onSave);
    connect(m_cancelBtn, &QPushButton::clicked, this, &BlockEditor::onCancel);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);
}

void BlockEditor::loadBlock()
{
    BlockDefinition *def = BlockManager::instance().block(m_blockName);
    if (!def) {
        QMessageBox::warning(this, "错误", QString("块 '%1' 不存在").arg(m_blockName));
        return;
    }

    // 创建工作副本
    m_workingCopy = def->clone();

    // 加载图元到场景（简化：只添加基本图元）
    m_scene->clear();
    for (QGraphicsItem *item : def->items()) {
        // 简化：不深拷贝，只显示边界
        // 实际应该深拷贝每个图元
        Q_UNUSED(item);
    }

    // 添加一些示例图元用于演示编辑
    LineItem *line = new LineItem(QPointF(-50, 0), QPointF(50, 0));
    m_scene->addItem(line);

    CircleItem *circle = new CircleItem(QPointF(0, 30), 20);
    m_scene->addItem(circle);

    refreshAttributeList();
    m_view->zoomExtents();
}

void BlockEditor::saveBlock()
{
    BlockDefinition *def = BlockManager::instance().block(m_blockName);
    if (!def) return;

    // 从场景读取图元更新块定义
    def->clearItems();
    for (QGraphicsItem *item : m_scene->items()) {
        if (dynamic_cast<LineItem*>(item) || dynamic_cast<CircleItem*>(item)) {
            def->addItem(item);
        }
    }

    // 更新属性定义（从工作副本）
    if (m_workingCopy) {
        for (const auto &attr : m_workingCopy->attributes()) {
            def->addAttribute(attr);
        }
    }

    // 触发重定义
    BlockManager::instance().redefineBlock(m_blockName, def);
    emit blockEdited(m_blockName);
    accept();
}

void BlockEditor::refreshAttributeList()
{
    m_attrList->clear();
    if (!m_workingCopy) return;

    for (const auto &attr : m_workingCopy->attributes()) {
        QString itemText = QString("%1 (%2) - 默认: %3")
            .arg(attr.tag)
            .arg(attr.visible ? "可见" : "隐藏")
            .arg(attr.defaultValue.isEmpty() ? "(空)" : attr.defaultValue);
        QListWidgetItem *item = new QListWidgetItem(itemText, m_attrList);
        item->setData(Qt::UserRole, attr.tag);
    }
}

void BlockEditor::onSave()
{
    if (QMessageBox::question(this, "保存", "确定保存块定义吗？所有引用将自动更新。") == QMessageBox::Yes) {
        saveBlock();
    }
}

void BlockEditor::onCancel()
{
    reject();
}

void BlockEditor::onAddAttribute()
{
    if (!m_workingCopy) return;

    bool ok;
    QString tag = QInputDialog::getText(this, "添加属性", "属性标记:", QLineEdit::Normal, "", &ok);
    if (!ok || tag.isEmpty()) return;

    if (m_workingCopy->hasAttribute(tag)) {
        QMessageBox::warning(this, "错误", "属性标记已存在");
        return;
    }

    QString prompt = QInputDialog::getText(this, "添加属性", "提示文字:", QLineEdit::Normal, "请输入" + tag, &ok);
    if (!ok) return;

    QString defaultValue = QInputDialog::getText(this, "添加属性", "默认值:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    AttributeDefinition attr;
    attr.tag = tag;
    attr.prompt = prompt;
    attr.defaultValue = defaultValue;
    attr.visible = true;
    attr.position = QPointF(0, m_workingCopy->attributeCount() * 15);
    m_workingCopy->addAttribute(attr);
    refreshAttributeList();
}

void BlockEditor::onEditAttribute()
{
    if (!m_workingCopy) return;
    QListWidgetItem *item = m_attrList->currentItem();
    if (!item) return;

    QString tag = item->data(Qt::UserRole).toString();
    AttributeDefinition *attr = m_workingCopy->attribute(tag);
    if (!attr) return;

    bool ok;
    QString newPrompt = QInputDialog::getText(this, "编辑属性", "提示文字:", QLineEdit::Normal, attr->prompt, &ok);
    if (!ok) return;

    QString newDefault = QInputDialog::getText(this, "编辑属性", "默认值:", QLineEdit::Normal, attr->defaultValue, &ok);
    if (!ok) return;

    attr->prompt = newPrompt;
    attr->defaultValue = newDefault;
    refreshAttributeList();
}

void BlockEditor::onDeleteAttribute()
{
    if (!m_workingCopy) return;
    QListWidgetItem *item = m_attrList->currentItem();
    if (!item) return;

    QString tag = item->data(Qt::UserRole).toString();
    if (QMessageBox::question(this, "删除属性", QString("确定删除属性 '%1' 吗？").arg(tag)) == QMessageBox::Yes) {
        m_workingCopy->removeAttribute(tag);
        refreshAttributeList();
    }
}

void BlockEditor::onAddLine()
{
    LineItem *line = new LineItem(QPointF(0, 0), QPointF(50, 0));
    m_scene->addItem(line);
    m_view->zoomExtents();
}

void BlockEditor::onAddCircle()
{
    CircleItem *circle = new CircleItem(QPointF(0, 0), 25);
    m_scene->addItem(circle);
    m_view->zoomExtents();
}

void BlockEditor::onAddText()
{
    bool ok;
    QString text = QInputDialog::getText(this, "添加文字", "文字内容:", QLineEdit::Normal, "文字", &ok);
    if (!ok || text.isEmpty()) return;

    TextItem *textItem = new TextItem(text);
    textItem->setPos(0, 0);
    m_scene->addItem(textItem);
    m_view->zoomExtents();
}

void BlockEditor::onDeleteSelected()
{
    auto items = m_scene->selectedItems();
    for (auto item : items) {
        m_scene->removeItem(item);
        delete item;
    }
}

void BlockEditor::onZoomExtents()
{
    m_view->zoomExtents();
}

} // namespace Zhifen
