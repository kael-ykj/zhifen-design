#include "layerpanel.h"
#include "document.h"
#include <QHBoxLayout>
#include <QInputDialog>
#include <QColorDialog>

LayerPanel::LayerPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    m_layerList = new QListWidget(this);
    m_layerList->setStyleSheet("background: #252526; color: #ccc; border: 1px solid #3c3c3c; font-size: 12px;");

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("新建", this);
    m_deleteBtn = new QPushButton("删除", this);
    m_addBtn->setStyleSheet("background: #0e639c; color: white; border: none; padding: 4px 8px; font-size: 11px;");
    m_deleteBtn->setStyleSheet("background: #0e639c; color: white; border: none; padding: 4px 8px; font-size: 11px;");
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_deleteBtn);

    layout->addWidget(m_layerList);
    layout->addLayout(btnLayout);

    connect(m_layerList, &QListWidget::itemClicked, this, &LayerPanel::onItemClicked);
    connect(m_addBtn, &QPushButton::clicked, this, &LayerPanel::onAddLayer);
    connect(m_deleteBtn, &QPushButton::clicked, this, &LayerPanel::onDeleteLayer);
}

void LayerPanel::setDocument(Document *doc)
{
    m_document = doc;
    refresh();
}

void LayerPanel::refresh()
{
    m_layerList->clear();
    if (!m_document) return;
    for (const LayerInfo &layer : m_document->getAllLayers()) {
        QListWidgetItem *item = new QListWidgetItem(m_layerList);
        item->setText(layer.name);
        item->setForeground(layer.color);
        if (layer.name == m_document->currentLayer()) {
            item->setBackground(QColor(9, 71, 113));
        }
        m_layerList->addItem(item);
    }
}

void LayerPanel::onItemClicked(QListWidgetItem *item)
{
    if (!m_document || !item) return;
    m_document->setCurrentLayer(item->text());
    emit currentLayerChanged(item->text());
    refresh();
}

void LayerPanel::onAddLayer()
{
    if (!m_document) return;
    bool ok;
    QString name = QInputDialog::getText(this, "新建图层", "图层名称:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        QColor color = QColorDialog::getColor(Qt::white, this, "选择图层颜色");
        if (color.isValid()) {
            m_document->addLayer(name, color);
            refresh();
        }
    }
}

void LayerPanel::onDeleteLayer()
{
    if (!m_document || !m_layerList->currentItem()) return;
    m_document->removeLayer(m_layerList->currentItem()->text());
    refresh();
}
