#include "blockmanagerpanel.h"
#include "blocks/blockdefinition.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QListWidgetItem>

namespace Zhifen {

BlockManagerPanel::BlockManagerPanel(QWidget *parent)
    : QDockWidget("块管理器", parent)
{
    setupUI();
    refresh();
}

void BlockManagerPanel::setupUI()
{
    setStyleSheet("QDockWidget { background: #252526; color: #ccc; } QDockWidget::title { background: #2d2d30; padding: 6px; } QListWidget { background: #1e1e1e; color: #ccc; border: 1px solid #3c3c3c; } QListWidget::item:selected { background: #0e639c; } QLineEdit { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 4px; } QComboBox { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 2px; } QPushButton { background: #0e639c; color: #fff; border: none; padding: 6px 12px; } QPushButton:hover { background: #1177bb; } QPushButton:disabled { background: #3c3c3c; color: #888; } QLabel { color: #999; }");

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // 搜索和筛选
    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索块...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BlockManagerPanel::onSearch);
    filterLayout->addWidget(m_searchEdit);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("全部");
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BlockManagerPanel::onCategoryChanged);
    filterLayout->addWidget(m_categoryCombo);
    mainLayout->addLayout(filterLayout);

    // 块列表
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, &BlockManagerPanel::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &BlockManagerPanel::onItemSelectionChanged);
    mainLayout->addWidget(m_listWidget, 1);

    // 信息标签
    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    mainLayout->addWidget(m_infoLabel);

    // 操作按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_insertBtn = new QPushButton("插入", this);
    m_editBtn = new QPushButton("编辑", this);
    m_renameBtn = new QPushButton("重命名", this);
    m_deleteBtn = new QPushButton("删除", this);
    connect(m_insertBtn, &QPushButton::clicked, this, &BlockManagerPanel::onInsert);
    connect(m_editBtn, &QPushButton::clicked, this, &BlockManagerPanel::onEdit);
    connect(m_renameBtn, &QPushButton::clicked, this, &BlockManagerPanel::onRename);
    connect(m_deleteBtn, &QPushButton::clicked, this, &BlockManagerPanel::onDelete);
    btnLayout->addWidget(m_insertBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_renameBtn);
    btnLayout->addWidget(m_deleteBtn);
    mainLayout->addLayout(btnLayout);

    setWidget(contentWidget);
    onItemSelectionChanged();
}

void BlockManagerPanel::refresh()
{
    // 更新类别
    QString currentCat = m_categoryCombo->currentText();
    m_categoryCombo->blockSignals(true);
    m_categoryCombo->clear();
    m_categoryCombo->addItem("全部");
    for (const QString &cat : BlockManager::instance().categories()) {
        m_categoryCombo->addItem(cat);
    }
    int idx = m_categoryCombo->findText(currentCat);
    m_categoryCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    m_categoryCombo->blockSignals(false);

    updateList();
}

void BlockManagerPanel::updateList()
{
    m_listWidget->clear();
    QString searchText = m_searchEdit->text().toLower();
    QString category = m_categoryCombo->currentText();

    for (const QString &name : BlockManager::instance().allBlockNames()) {
        // 类别筛选
        if (category != "全部") {
            QStringList blocks = BlockManager::instance().blocksByCategory(category);
            if (!blocks.contains(name)) continue;
        }
        // 搜索筛选
        if (!searchText.isEmpty() && !name.toLower().contains(searchText)) continue;

        BlockDefinition *def = BlockManager::instance().block(name);
        int refCount = BlockManager::instance().referenceCount(name);
        QString itemText = QString("%1  (%2图元, %3属性, %4引用)")
            .arg(name)
            .arg(def ? def->itemCount() : 0)
            .arg(def ? def->attributeCount() : 0)
            .arg(refCount);
        QListWidgetItem *item = new QListWidgetItem(itemText, m_listWidget);
        item->setData(Qt::UserRole, name);
    }
}

QString BlockManagerPanel::currentBlockName() const
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return QString();
    return item->data(Qt::UserRole).toString();
}

void BlockManagerPanel::onInsert()
{
    QString name = currentBlockName();
    if (!name.isEmpty()) {
        emit insertBlockRequested(name);
    }
}

void BlockManagerPanel::onEdit()
{
    QString name = currentBlockName();
    if (!name.isEmpty()) {
        emit editBlockRequested(name);
    }
}

void BlockManagerPanel::onRename()
{
    QString name = currentBlockName();
    if (name.isEmpty()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "重命名块", "输入新名称:", QLineEdit::Normal, name, &ok);
    if (ok && !newName.isEmpty() && newName != name) {
        if (BlockManager::instance().renameBlock(name, newName)) {
            refresh();
        } else {
            QMessageBox::warning(this, "重命名失败", "名称已存在或重命名失败");
        }
    }
}

void BlockManagerPanel::onDelete()
{
    QString name = currentBlockName();
    if (name.isEmpty()) return;

    int refCount = BlockManager::instance().referenceCount(name);
    if (refCount > 0) {
        QMessageBox::warning(this, "删除失败", QString("块 '%1' 被 %2 个引用使用，无法删除").arg(name).arg(refCount));
        return;
    }

    if (QMessageBox::question(this, "删除块", QString("确定要删除块 '%1' 吗？").arg(name)) == QMessageBox::Yes) {
        if (BlockManager::instance().removeBlock(name)) {
            refresh();
        }
    }
}

void BlockManagerPanel::onSearch(const QString &text)
{
    Q_UNUSED(text);
    updateList();
}

void BlockManagerPanel::onCategoryChanged(int index)
{
    Q_UNUSED(index);
    updateList();
}

void BlockManagerPanel::onItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    QString name = item->data(Qt::UserRole).toString();
    if (!name.isEmpty()) {
        emit insertBlockRequested(name);
    }
}

void BlockManagerPanel::onItemSelectionChanged()
{
    QString name = currentBlockName();
    bool hasSelection = !name.isEmpty();
    m_insertBtn->setEnabled(hasSelection);
    m_editBtn->setEnabled(hasSelection);
    m_renameBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);

    if (hasSelection) {
        BlockDefinition *def = BlockManager::instance().block(name);
        if (def) {
            m_infoLabel->setText(QString("名称: %1\n描述: %2\n图元: %3  属性: %4  引用: %5")
                .arg(def->name())
                .arg(def->description().isEmpty() ? "无" : def->description())
                .arg(def->itemCount())
                .arg(def->attributeCount())
                .arg(BlockManager::instance().referenceCount(name)));
        }
    } else {
        m_infoLabel->setText("选择一个块查看详情");
    }
}

} // namespace Zhifen
