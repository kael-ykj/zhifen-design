#include "layerdialog.h"
#include "cad/document.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>

LayerDialog::LayerDialog(Document *doc, QWidget *parent)
    : QDialog(parent), m_document(doc)
{
    setWindowTitle("图层特性管理器");
    resize(700, 500);
    setupUI();
    loadLayers();
}

void LayerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels({"状态", "名称", "颜色", "线型", "线宽", "冻结/锁定/打印"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    connect(m_table, &QTableWidget::itemChanged, this, &LayerDialog::onItemChanged);
    mainLayout->addWidget(m_table);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_newBtn = new QPushButton("新建图层", this);
    m_delBtn = new QPushButton("删除图层", this);
    m_currentBtn = new QPushButton("置为当前", this);
    m_okBtn = new QPushButton("确定", this);
    connect(m_newBtn, &QPushButton::clicked, this, &LayerDialog::onNewLayer);
    connect(m_delBtn, &QPushButton::clicked, this, &LayerDialog::onDeleteLayer);
    connect(m_currentBtn, &QPushButton::clicked, this, &LayerDialog::onSetCurrent);
    connect(m_okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(m_newBtn);
    btnLayout->addWidget(m_delBtn);
    btnLayout->addWidget(m_currentBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_okBtn);
    mainLayout->addLayout(btnLayout);
}

void LayerDialog::loadLayers()
{
    m_table->blockSignals(true);
    m_table->setRowCount(0);
    if (!m_document) return;

    QList<LayerInfo> layers = m_document->getAllLayers();
    QString current = m_document->currentLayer();

    for (const LayerInfo &layer : layers) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // 状态（当前层标记）
        QTableWidgetItem *statusItem = new QTableWidgetItem(layer.name == current ? "✓ 当前" : "");
        statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, statusItem);

        // 名称
        QTableWidgetItem *nameItem = new QTableWidgetItem(layer.name);
        if (layer.name == "0" || layer.name == "DEFPOINTS")
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 1, nameItem);

        // 颜色
        QTableWidgetItem *colorItem = new QTableWidgetItem(layer.color.name());
        colorItem->setBackground(layer.color);
        colorItem->setForeground(layer.color.lightness() > 128 ? Qt::black : Qt::white);
        colorItem->setFlags(colorItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 2, colorItem);

        // 线型
        QTableWidgetItem *ltypeItem = new QTableWidgetItem(layer.lineType);
        m_table->setItem(row, 3, ltypeItem);

        // 线宽
        QTableWidgetItem *lwItem = new QTableWidgetItem(QString::number(layer.lineWidth));
        m_table->setItem(row, 4, lwItem);

        // 冻结/锁定/打印
        QString flags = QString("%1%2%3")
            .arg(layer.frozen ? "冻 " : "")
            .arg(layer.locked ? "锁 " : "")
            .arg(layer.plot ? "" : "不打印");
        QTableWidgetItem *flagItem = new QTableWidgetItem(flags.trimmed());
        flagItem->setFlags(flagItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 5, flagItem);
    }
    m_table->blockSignals(false);
}

void LayerDialog::onNewLayer()
{
    if (!m_document) return;
    bool ok;
    QString name = QInputDialog::getText(this, "新建图层", "图层名称:", QLineEdit::Normal, "图层1", &ok);
    if (ok && !name.isEmpty()) {
        QColor color = QColorDialog::getColor(Qt::white, this, "选择图层颜色");
        if (color.isValid()) {
            m_document->addLayer(name, color);
            loadLayers();
        }
    }
}

void LayerDialog::onDeleteLayer()
{
    if (!m_document || m_table->currentRow() < 0) return;
    QString name = m_table->item(m_table->currentRow(), 1)->text();
    if (name == "0" || name == "DEFPOINTS") {
        QMessageBox::warning(this, "警告", "不能删除0层和DEFPOINTS层！");
        return;
    }
    if (QMessageBox::question(this, "确认", QString("确定删除图层 \"%1\"？").arg(name)) == QMessageBox::Yes) {
        m_document->removeLayer(name);
        loadLayers();
    }
}

void LayerDialog::onSetCurrent()
{
    if (!m_document || m_table->currentRow() < 0) return;
    QString name = m_table->item(m_table->currentRow(), 1)->text();
    m_document->setCurrentLayer(name);
    loadLayers();
}

void LayerDialog::onItemChanged(QTableWidgetItem *item)
{
    if (!m_document || item->column() != 1) return;
    int row = item->row();
    QString oldName = m_table->item(row, 1)->text();
    // 重命名功能简化处理
}

void LayerDialog::onColorClicked()
{
    // 双击颜色列时弹出颜色选择器
}

void LayerDialog::refresh()
{
    loadLayers();
}
