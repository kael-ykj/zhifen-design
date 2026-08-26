#include "layerpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>

LayerPanel::LayerPanel(QWidget *parent)
    : QDockWidget("图层管理", parent)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);

    QLabel* title = new QLabel("图层列表 (单击切换可见, 双击改颜色)", this);
    title->setStyleSheet("font-size: 11px; color: #666;");
    layout->addWidget(title);

    m_layerList = new QListWidget(this);
    m_layerList->setAlternatingRowColors(true);
    m_layerList->setStyleSheet("QListWidget::item { height: 24px; }");
    layout->addWidget(m_layerList);

    connect(m_layerList, &QListWidget::itemClicked, this, &LayerPanel::onItemClicked);
    connect(m_layerList, &QListWidget::itemDoubleClicked, this, &LayerPanel::onItemDoubleClicked);

    setWidget(container);
    setMinimumWidth(200);
}

void LayerPanel::setProject(zf::Project* project)
{
    m_project = project;
    refresh();
}

void LayerPanel::refresh()
{
    if (!m_project) return;
    m_layerList->clear();
    for (const auto& layer : m_project->layers) {
        QListWidgetItem* item = new QListWidgetItem(m_layerList);
        item->setText(QString::fromStdString(layer.name));
        item->setData(Qt::UserRole, QString::fromStdString(layer.layerId));
        updateItemIcon(item, layer);
        item->setToolTip(QString("可见:%1 锁定:%2 冻结:%3")
            .arg(layer.visible ? "是" : "否")
            .arg(layer.locked ? "是" : "否")
            .arg(layer.frozen ? "是" : "否"));
    }
}

void LayerPanel::updateItemIcon(QListWidgetItem* item, const zf::Layer& layer)
{
    // 用文字前缀表示状态：[眼]可见 [锁]锁定 [雪]冻结
    QString prefix;
    prefix += layer.visible ? "[开]" : "[关]";
    prefix += layer.locked ? "[锁]" : "    ";
    prefix += layer.frozen ? "[冻]" : "    ";
    item->setText(prefix + "  " + QString::fromStdString(layer.name));

    // 文字颜色表示图层颜色
    QColor c = layerColor(layer.color);
    item->setForeground(c);
}

QColor LayerPanel::layerColor(int rgb) const
{
    return QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void LayerPanel::onItemClicked(QListWidgetItem* item)
{
    if (!m_project || !item) return;
    QString layerId = item->data(Qt::UserRole).toString();
    for (auto& layer : m_project->layers) {
        if (layer.layerId == layerId.toStdString()) {
            layer.visible = !layer.visible;
            updateItemIcon(item, layer);
            emit layerChanged();
            return;
        }
    }
}

void LayerPanel::onItemDoubleClicked(QListWidgetItem* item)
{
    if (!m_project || !item) return;
    QString layerId = item->data(Qt::UserRole).toString();
    for (auto& layer : m_project->layers) {
        if (layer.layerId == layerId.toStdString()) {
            QColor current = layerColor(layer.color);
            QColor newColor = QColorDialog::getColor(current, this,
                QString("选择图层颜色 - %1").arg(QString::fromStdString(layer.name)));
            if (newColor.isValid()) {
                layer.color = (newColor.red() << 16) | (newColor.green() << 8) | newColor.blue();
                updateItemIcon(item, layer);
                emit layerChanged();
            }
            return;
        }
    }
}
