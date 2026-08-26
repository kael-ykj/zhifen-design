#pragma once
#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "core/zf_types.h"

class LayerPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit LayerPanel(QWidget *parent = nullptr);
    void setProject(zf::Project* project);
    void refresh();

signals:
    void layerChanged();

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void updateItemIcon(QListWidgetItem* item, const zf::Layer& layer);
    QColor layerColor(int rgb) const;

    QListWidget* m_layerList{nullptr};
    zf::Project* m_project{nullptr};
};
