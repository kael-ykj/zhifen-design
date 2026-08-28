#include "core_api.h"
#include "../devices/deviceitem.h"
#include "../entities/caditem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>

namespace Zhifen {

CoreApi::CoreApi(QGraphicsScene *scene) : m_scene(scene) {}
CoreApi::~CoreApi() {}

QList<DeviceItem*> CoreApi::getAllDevices() const {
    QList<DeviceItem*> result;
    if (!m_scene) return result;
    for (auto *item : m_scene->items()) {
        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            result.append(dev);
        }
    }
    return result;
}

QList<DeviceItem*> CoreApi::getSelectedDevices() const {
    QList<DeviceItem*> result;
    if (!m_scene) return result;
    for (auto *item : m_scene->selectedItems()) {
        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            result.append(dev);
        }
    }
    return result;
}

DeviceItem* CoreApi::addDevice(const QString &typeName, const QPointF &pos) {
    if (!m_scene) return nullptr;
    DeviceItem *device = new DeviceItem();
    device->setPos(pos);
    m_scene->addItem(device);
    return device;
}

bool CoreApi::removeDevice(DeviceItem *device) {
    if (!m_scene || !device) return false;
    m_scene->removeItem(device);
    delete device;
    return true;
}

int CoreApi::renameDevices(const QList<DeviceItem*> &devices, const QString &prefix, int startIndex) {
    int count = 0;
    for (auto *dev : devices) {
        if (!dev) continue;
        QString newName = QString("%1-%2").arg(prefix).arg(startIndex + count, 3, 10, QChar('0'));
        dev->setModel(newName);
        count++;
    }
    return count;
}

int CoreApi::moveDevices(const QList<DeviceItem*> &devices, const QPointF &offset) {
    int count = 0;
    for (auto *dev : devices) {
        if (!dev) continue;
        dev->moveBy(offset.x(), offset.y());
        count++;
    }
    return count;
}

int CoreApi::rotateDevices(const QList<DeviceItem*> &devices, qreal angle) {
    int count = 0;
    for (auto *dev : devices) {
        if (!dev) continue;
        dev->setRotation(dev->rotation() + angle);
        count++;
    }
    return count;
}

QList<DeviceItem*> CoreApi::findDevicesByType(const QString &typeName) const {
    QList<DeviceItem*> result;
    for (auto *dev : getAllDevices()) {
        if (dev->deviceTypeName().contains(typeName, Qt::CaseInsensitive)) {
            result.append(dev);
        }
    }
    return result;
}

QList<DeviceItem*> CoreApi::findDevicesByName(const QString &name) const {
    QList<DeviceItem*> result;
    for (auto *dev : getAllDevices()) {
        if (dev->model().contains(name, Qt::CaseInsensitive)) {
            result.append(dev);
        }
    }
    return result;
}

bool CoreApi::executeCommand(const QString &command, const QVariantMap &args) {
    Q_UNUSED(args);
    // 简化实现：记录命令
    log(QString("执行命令: %1").arg(command));
    return true;
}

void CoreApi::log(const QString &message) {
    qDebug() << "[Plugin]" << message;
}

} // namespace Zhifen
