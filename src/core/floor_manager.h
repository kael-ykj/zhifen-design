#ifndef FLOOR_MANAGER_H
#define FLOOR_MANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QGraphicsScene>
#include <QGraphicsItem>

namespace Zhifen {

// 楼层信息
struct FloorInfo {
    int id = 0;              // 楼层ID
    QString name;            // 楼层名称（如"1F"、"2F"、"标准层"）
    int floorNumber = 1;     // 楼层编号
    qreal height = 3.0;      // 层高（米）
    bool isStandard = false; // 是否为标准层
    QString description;     // 备注
};

// 复制选项
enum CopyOption {
    Copy_All = 0,       // 复制全部
    Copy_Devices = 1,   // 仅复制器件
    Copy_Feeders = 2,   // 仅复制馈线
    Copy_Other = 3      // 仅复制其他
};

// 楼层管理器
class FloorManager
{
public:
    FloorManager();
    ~FloorManager();

    // 获取单例
    static FloorManager& instance();

    // 楼层管理
    int addFloor(const QString &name = "", int floorNumber = 0);
    bool removeFloor(int floorId);
    bool renameFloor(int floorId, const QString &name);
    bool setFloorNumber(int floorId, int number);
    bool setFloorHeight(int floorId, qreal height);
    bool setStandardFloor(int floorId, bool isStandard);

    // 楼层切换
    bool switchToFloor(int floorId, QGraphicsScene *scene);
    int currentFloorId() const { return m_currentFloorId; }
    FloorInfo currentFloor() const;

    // 标准层复制
    bool copyStandardToFloor(int sourceFloorId, int targetFloorId, QGraphicsScene *scene, CopyOption option = Copy_All);
    bool copyFloorToFloor(int sourceFloorId, int targetFloorId, QGraphicsScene *scene, CopyOption option = Copy_All);

    // 查询
    QList<FloorInfo> allFloors() const { return m_floors; }
    FloorInfo floorInfo(int floorId) const;
    QList<int> standardFloors() const;
    int floorCount() const { return m_floors.size(); }

    // 保存/加载当前楼层状态
    void saveCurrentFloorState(QGraphicsScene *scene);
    void loadFloorState(int floorId, QGraphicsScene *scene);

    // 清理
    void clear();

private:
    QList<FloorInfo> m_floors;
    int m_currentFloorId = 0;
    int m_nextFloorId = 1;

    // 每个楼层的图元数据（序列化存储）
    QMap<int, QList<QGraphicsItem*>> m_floorItems;

    // 深拷贝图元
    QList<QGraphicsItem*> cloneItems(const QList<QGraphicsItem*> &items, CopyOption option);

    // 过滤图元类型
    bool shouldCopyItem(QGraphicsItem *item, CopyOption option);
};

} // namespace Zhifen

#endif // FLOOR_MANAGER_H
