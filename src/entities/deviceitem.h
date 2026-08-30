#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QPointF>
#include <QRectF>
#include "device_symbols.h"

namespace Zhifen {

// 专业室分器件图元
// 使用行业标准画法，对标天越/AIDP
class DeviceItem : public QGraphicsItem
{
public:
    enum DeviceType {
        // 天线类
        OmniAntenna,
        DirectionalAntenna,
        SpotlightAntenna,
        ExternalAntenna,
        WallMountAntenna,
        // 器件类
        Coupler,
        Splitter,
        Combiner,
        Hybrid,
        Attenuator,
        Load,
        Isolator,
        Circulator,
        // 信源类
        MacroBS,
        MicroBS,
        FiberRepeater,
        BBU,
        RRU,
        PicoStation,
        // 其他
        Ground,
        LightningProtector,
        Switch,
        Connector
    };

    enum { Type = UserType + 200 };

    explicit DeviceItem(DeviceType type = OmniAntenna, QGraphicsItem *parent = nullptr);
    virtual ~DeviceItem();

    // 器件类型
    DeviceType deviceType() const { return m_type; }
    void setDeviceType(DeviceType type) { m_type = type; prepareGeometryChange(); update(); }

    // 器件名称
    QString deviceName() const;
    static QString deviceTypeName(DeviceType type);

    // 参数
    int couplerDb() const { return m_couplerDb; }
    void setCouplerDb(int db) { m_couplerDb = db; update(); }

    int splitterWays() const { return m_splitterWays; }
    void setSplitterWays(int ways) { m_splitterWays = ways; prepareGeometryChange(); update(); }

    int attenuatorDb() const { return m_attenuatorDb; }
    void setAttenuatorDb(int db) { m_attenuatorDb = db; update(); }

    // 缩放
    qreal symbolScale() const { return m_scale; }
    void setSymbolScale(qreal s) { m_scale = s; prepareGeometryChange(); update(); }

    // 连接点
    QList<QPointF> connectionPoints() const;

    // QGraphicsItem接口
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return Type; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    DeviceType m_type;
    qreal m_scale = 1.0;
    int m_couplerDb = 10;
    int m_splitterWays = 2;
    int m_attenuatorDb = 10;
};

} // namespace Zhifen

#endif // DEVICEITEM_H
