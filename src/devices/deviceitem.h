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
    DevAntennaOmni,       // 全向吸顶天线
    DevAntennaDirectional, // 定向壁挂天线
    DevAntennaLPDA,       // 对数周期天线
    DevSplitter2,         // 二功分器
    DevSplitter3,         // 三功分器
    DevSplitter4,         // 四功分器
    DevCoupler5,          // 5dB耦合器
    DevCoupler7,          // 7dB耦合器
    DevCoupler10,         // 10dB耦合器
    DevCoupler15,         // 15dB耦合器
    DevCoupler20,         // 20dB耦合器
    DevCombiner,          // 合路器
    DevFeederHalf,        // 1/2馈线
    DevFeeder78,          // 7/8馈线
    DevFiber,             // 光纤
    DevSourceRRU,         // RRU
    DevSourceBBU,         // BBU
    DevSourceMicro,       // 微基站
    DevSourceRepeater     // 直放站
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
