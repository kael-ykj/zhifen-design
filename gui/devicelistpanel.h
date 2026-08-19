#pragma once

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include "device/device_library.h"

class DeviceListPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceListPanel(zf::DeviceLibrary* devLib, QWidget *parent = nullptr);

signals:
    void deviceModelSelected(const QString& modelId);

private slots:
    void onCategoryChanged(int index);
    void onDeviceClicked(QListWidgetItem* item);

private:
    void refreshList();

    zf::DeviceLibrary* m_devLib{nullptr};
    QComboBox* m_categoryCombo{nullptr};
    QListWidget* m_deviceList{nullptr};
    QLabel* m_infoLabel{nullptr};
};
