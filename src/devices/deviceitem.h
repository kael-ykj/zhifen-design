#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include "entities/caditem.h"
#include <QMap>
#include <QString>
#include <QPointF>
#include <QList>

// 器件类型枚举
enum DeviceType {
    DevUnknown = 0,
    // === 天线类 ===
    DevAntennaOmni,       // 全向吸顶天线
    DevAntennaDirectional, // 定向壁挂天线
    DevAntennaLPDA,       // 对数周期天线
    DevAntennaSpotlight,  // 射灯天线(楼间对打)
    DevAntennaExternal,   // 外引天线
    DevAntennaPanel,      // 板状天线
    DevAntennaYagi,       // 八木天线
    DevAntennaGrid,       // 栅格天线
    // === 功分器类 ===
    DevSplitter2,         // 二功分器
    DevSplitter3,         // 三功分器
    DevSplitter4,         // 四功分器
    // === 耦合器类 ===
    DevCoupler5,          // 5dB耦合器
    DevCoupler7,          // 7dB耦合器
    DevCoupler10,         // 10dB耦合器
    DevCoupler15,         // 15dB耦合器
    DevCoupler20,         // 20dB耦合器
    DevCoupler25,         // 25dB耦合器
    DevCoupler30,         // 30dB耦合器
    // === 其他无源器件 ===
    DevCombiner,          // 合路器
    DevHybrid,            // 3dB电桥
    DevLoad,              // 终端负载
    DevAttenuator,        // 衰减器
    DevLightning,         // 避雷器
    // === 馈线类 ===
    DevFeederHalf,        // 1/2馈线
    DevFeeder78,          // 7/8馈线
    DevFeeder158,         // 1-5/8馈线
    DevFeeder5D,          // 5D-FB馈线
    DevFeeder8D,          // 8D-FB馈线
    DevFiber,             // 光纤
    DevNetworkCable,      // 网线(CAT6)
    // === 漏缆类 ===
    DevLeakyCable158,     // 1-5/8漏缆
    DevLeakyCable138,     // 13/8漏缆
    // === 信源类(传统) ===
    DevSourceRRU,         // RRU
    DevSourceBBU,         // BBU
    DevSourceMicro,       // 微基站
    DevSourceRepeater,    // 直放站
    DevDryAmp,            // 干放
    // === 数字化室分类 ===
    DevpRRU,              // pRRU皮基站
    DevRHUB,              // RHUB射频集线器
    DevPOESwitch          // POE交换机
};

// 连接点结构
struct ConnectPoint {
    QPointF pos;
    QString name;
    bool connected = false;
};

class DeviceItem : public CadItem
{
public:
    explicit DeviceItem(DeviceType type = DevUnknown, QGraphicsItem *parent = nullptr);

    DeviceType deviceType() const { return m_deviceType; }
    QString deviceTypeName() const;
    QString model() const { return m_model; }
    void setModel(const QString &model) { m_model = model; }

    // 器件属性
    QMap<QString, QString> properties() const { return m_properties; }
    void setProperty(const QString &key, const QString &value) { m_properties[key] = value; }
    QString property(const QString &key) const { return m_properties.value(key, ""); }

    // 连接点
    QList<ConnectPoint> connectPoints() const { return m_connectPoints; }
    int connectPointCount() const { return m_connectPoints.size(); }

    // 损耗（dB）
    virtual qreal insertionLoss() const { return m_insertionLoss; }
    void setInsertionLoss(qreal loss) { m_insertionLoss = loss; }

    // 功率（dBm）
    qreal power() const { return m_power; }
    void setPower(qreal power) { m_power = power; }

    QString entityType() const override { return deviceTypeName(); }
    QPointF center() const override { return pos(); }
    qreal distanceToPoint(const QPointF &pos) const override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    DeviceType m_deviceType = DevUnknown;
    QString m_model;
    QMap<QString, QString> m_properties;
    QList<ConnectPoint> m_connectPoints;
    qreal m_insertionLoss = 0.0;
    qreal m_power = 0.0;
    qreal m_size = 10.0; // 图元尺寸

    void initDeviceProperties();
    void setupConnectPoints();
    QPen devicePen() const;
};

#endif // DEVICEITEM_H
