#ifndef CORE_API_H
#define CORE_API_H

#include <QString>
#include <QList>
#include <QPointF>
#include <QGraphicsScene>

namespace Zhifen {

class DeviceItem;
class CadItem;

// 核心API - 暴露给插件使用
class CoreApi
{
public:
    CoreApi(QGraphicsScene *scene = nullptr);
    ~CoreApi();

    // 场景访问
    QGraphicsScene* scene() const { return m_scene; }
    void setScene(QGraphicsScene *scene) { m_scene = scene; }

    // 器件操作
    QList<DeviceItem*> getAllDevices() const;
    QList<DeviceItem*> getSelectedDevices() const;
    DeviceItem* addDevice(const QString &typeName, const QPointF &pos);
    bool removeDevice(DeviceItem *device);

    // 批量操作
    int renameDevices(const QList<DeviceItem*> &devices, const QString &prefix, int startIndex = 1);
    int moveDevices(const QList<DeviceItem*> &devices, const QPointF &offset);
    int rotateDevices(const QList<DeviceItem*> &devices, qreal angle);

    // 查询
    QList<DeviceItem*> findDevicesByType(const QString &typeName) const;
    QList<DeviceItem*> findDevicesByName(const QString &name) const;

    // 命令执行
    bool executeCommand(const QString &command, const QVariantMap &args = QVariantMap());

    // 日志
    void log(const QString &message);

private:
    QGraphicsScene *m_scene = nullptr;
};

} // namespace Zhifen

#endif // CORE_API_H
