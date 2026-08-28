#ifndef DEVICETOOL_H
#define DEVICETOOL_H

#include "tool.h"
#include "devices/deviceitem.h"
#include <QPointF>

class DeviceTool : public Tool
{
    Q_OBJECT
public:
    explicit DeviceTool(CadView *view, DeviceType type = DevAntennaOmni, QObject *parent = nullptr);
    QString name() const override { return "器件放置"; }
    void setDeviceType(DeviceType type) { m_deviceType = type; }
    DeviceType deviceType() const { return m_deviceType; }
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawOverlay(QPainter *painter) override;
private:
    DeviceType m_deviceType;
    QPointF m_currentPos;
};

#endif // DEVICETOOL_H
