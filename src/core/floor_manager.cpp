#include "floor_manager.h"
#include "../devices/deviceitem.h"
#include "../entities/feederitem.h"
#include "../entities/caditem.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QDebug>

namespace Zhifen {

FloorManager::FloorManager() {
    // 默认创建一个楼层
    FloorInfo floor;
    floor.id = 0;
    floor.name = "1F";
    floor.floorNumber = 1;
    floor.height = 3.0;
    floor.isStandard = false;
    m_floors.append(floor);
    m_currentFloorId = 0;
    m_nextFloorId = 1;
}

FloorManager::~FloorManager() {
    clear();
}

FloorManager& FloorManager::instance() {
    static FloorManager inst;
    return inst;
}

void FloorManager::clear() {
    for (auto &items : m_floorItems) {
        qDeleteAll(items);
    }
    m_floorItems.clear();
    m_floors.clear();
}

int FloorManager::addFloor(const QString &name, int floorNumber) {
    FloorInfo floor;
    floor.id = m_nextFloorId++;
    floor.name = name.isEmpty() ? QString("%1F").arg(floorNumber > 0 ? floorNumber : m_floors.size() + 1) : name;
    floor.floorNumber = floorNumber > 0 ? floorNumber : m_floors.size() + 1;
    floor.height = 3.0;
    floor.isStandard = false;
    m_floors.append(floor);
    return floor.id;
}

bool FloorManager::removeFloor(int floorId) {
    if (m_floors.size() <= 1) return false; // 至少保留一个楼层
    for (int i = 0; i < m_floors.size(); i++) {
        if (m_floors[i].id == floorId) {
            // 删除该楼层的图元
            if (m_floorItems.contains(floorId)) {
                qDeleteAll(m_floorItems[floorId]);
                m_floorItems.remove(floorId);
            }
            m_floors.removeAt(i);
            if (m_currentFloorId == floorId) {
                m_currentFloorId = m_floors[0].id;
            }
            return true;
        }
    }
    return false;
}

bool FloorManager::renameFloor(int floorId, const QString &name) {
    for (auto &floor : m_floors) {
        if (floor.id == floorId) {
            floor.name = name;
            return true;
        }
    }
    return false;
}

bool FloorManager::setFloorNumber(int floorId, int number) {
    for (auto &floor : m_floors) {
        if (floor.id == floorId) {
            floor.floorNumber = number;
            return true;
        }
    }
    return false;
}

bool FloorManager::setFloorHeight(int floorId, qreal height) {
    for (auto &floor : m_floors) {
        if (floor.id == floorId) {
            floor.height = height;
            return true;
        }
    }
    return false;
}

bool FloorManager::setStandardFloor(int floorId, bool isStandard) {
    for (auto &floor : m_floors) {
        if (floor.id == floorId) {
            floor.isStandard = isStandard;
            return true;
        }
    }
    return false;
}

FloorInfo FloorManager::currentFloor() const {
    return floorInfo(m_currentFloorId);
}

FloorInfo FloorManager::floorInfo(int floorId) const {
    for (const auto &floor : m_floors) {
        if (floor.id == floorId) return floor;
    }
    return FloorInfo();
}

QList<int> FloorManager::standardFloors() const {
    QList<int> result;
    for (const auto &floor : m_floors) {
        if (floor.isStandard) result.append(floor.id);
    }
    return result;
}

void FloorManager::saveCurrentFloorState(QGraphicsScene *scene) {
    if (!scene) return;
    // 清除旧的保存
    if (m_floorItems.contains(m_currentFloorId)) {
        qDeleteAll(m_floorItems[m_currentFloorId]);
    }
    m_floorItems[m_currentFloorId] = scene->items();
    // 从场景中移除但不删除
    for (auto *item : scene->items()) {
        scene->removeItem(item);
    }
}

void FloorManager::loadFloorState(int floorId, QGraphicsScene *scene) {
    if (!scene) return;
    // 清除当前场景
    scene->clear();
    // 加载目标楼层图元
    if (m_floorItems.contains(floorId)) {
        for (auto *item : m_floorItems[floorId]) {
            scene->addItem(item);
        }
    }
    m_currentFloorId = floorId;
}

bool FloorManager::switchToFloor(int floorId, QGraphicsScene *scene) {
    if (floorId == m_currentFloorId) return true;
    if (!scene) return false;

    // 保存当前楼层
    saveCurrentFloorState(scene);
    // 加载目标楼层
    loadFloorState(floorId, scene);
    return true;
}

bool FloorManager::shouldCopyItem(QGraphicsItem *item, CopyOption option) {
    if (option == Copy_All) return true;
    auto *cad = dynamic_cast<CadItem*>(item);
    if (!cad) return option == Copy_Other;

    if (option == Copy_Devices) {
        return dynamic_cast<DeviceItem*>(item) != nullptr;
    }
    if (option == Copy_Feeders) {
        return dynamic_cast<FeederItem*>(item) != nullptr;
    }
    if (option == Copy_Other) {
        return dynamic_cast<DeviceItem*>(item) == nullptr &&
               dynamic_cast<FeederItem*>(item) == nullptr;
    }
    return true;
}

QList<QGraphicsItem*> FloorManager::cloneItems(const QList<QGraphicsItem*> &items, CopyOption option) {
    QList<QGraphicsItem*> cloned;
    for (auto *item : items) {
        if (!shouldCopyItem(item, option)) continue;

        // 简化克隆：只支持DeviceItem和FeederItem
        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            DeviceItem *newDev = new DeviceItem(dev->deviceType());
            newDev->setPos(dev->pos());
            newDev->setRotation(dev->rotation());
            newDev->setModel(dev->model());
            cloned.append(newDev);
        } else if (auto *feeder = dynamic_cast<FeederItem*>(item)) {
            FeederItem *newFeeder = new FeederItem();
            newFeeder->setPos(feeder->pos());
            newFeeder->setRotation(feeder->rotation());
            cloned.append(newFeeder);
        }
    }
    return cloned;
}

bool FloorManager::copyFloorToFloor(int sourceFloorId, int targetFloorId, QGraphicsScene *scene, CopyOption option) {
    if (!scene) return false;
    if (sourceFloorId == targetFloorId) return false;

    // 获取源楼层图元
    QList<QGraphicsItem*> sourceItems;
    if (sourceFloorId == m_currentFloorId) {
        sourceItems = scene->items();
    } else if (m_floorItems.contains(sourceFloorId)) {
        sourceItems = m_floorItems[sourceFloorId];
    } else {
        return false;
    }

    // 克隆图元
    QList<QGraphicsItem*> cloned = cloneItems(sourceItems, option);

    // 如果目标是当前楼层，直接添加到场景
    if (targetFloorId == m_currentFloorId) {
        for (auto *item : cloned) {
            scene->addItem(item);
        }
    } else {
        // 保存到目标楼层
        if (m_floorItems.contains(targetFloorId)) {
            m_floorItems[targetFloorId].append(cloned);
        } else {
            m_floorItems[targetFloorId] = cloned;
        }
    }

    return true;
}

bool FloorManager::copyStandardToFloor(int sourceFloorId, int targetFloorId, QGraphicsScene *scene, CopyOption option) {
    // 检查源楼层是否为标准层
    bool isStandard = false;
    for (const auto &floor : m_floors) {
        if (floor.id == sourceFloorId && floor.isStandard) {
            isStandard = true;
            break;
        }
    }
    if (!isStandard) return false;

    return copyFloorToFloor(sourceFloorId, targetFloorId, scene, option);
}

} // namespace Zhifen
