#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include "devices/deviceitem.h"

class DevicePanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit DevicePanel(QWidget *parent = nullptr);
signals:
    void deviceSelected(DeviceType type);
    void placeDevice(DeviceType type);
private slots:
    void onItemClicked(QListWidgetItem *item);
    void onPlaceClicked();
private:
    QListWidget *m_list;
    QPushButton *m_placeBtn;
    DeviceType m_currentType = DevAntennaOmni;
    void setupUI();
    void loadDevices();
};

#endif // DEVICEPANEL_H
