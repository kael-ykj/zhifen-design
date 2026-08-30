#ifndef DEVICE_SYMBOLS_H
#define DEVICE_SYMBOLS_H

#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QString>

namespace Zhifen {

// 通信行业标准室分器件图元绘制引擎
// 参照 YD/T 5120-2015 室内分布系统设计规范
// 对标天越、AIDP等主流室分设计软件图元风格
class DeviceSymbols
{
public:
    // 天线类
    static void drawOmniAntenna(QPainter *painter, qreal scale = 1.0);
    static void drawDirectionalAntenna(QPainter *painter, qreal scale = 1.0);
    static void drawSpotlightAntenna(QPainter *painter, qreal scale = 1.0);
    static void drawExternalAntenna(QPainter *painter, qreal scale = 1.0);
    static void drawWallMountAntenna(QPainter *painter, qreal scale = 1.0);

    // 器件类
    static void drawCoupler(QPainter *painter, qreal scale = 1.0, int db = 10);
    static void drawSplitter(QPainter *painter, qreal scale = 1.0, int ways = 2);
    static void drawCombiner(QPainter *painter, qreal scale = 1.0);
    static void drawHybrid(QPainter *painter, qreal scale = 1.0);
    static void drawAttenuator(QPainter *painter, qreal scale = 1.0, int db = 10);
    static void drawLoad(QPainter *painter, qreal scale = 1.0);
    static void drawIsolator(QPainter *painter, qreal scale = 1.0);
    static void drawCirculator(QPainter *painter, qreal scale = 1.0);

    // 信源类
    static void drawMacroBS(QPainter *painter, qreal scale = 1.0);
    static void drawMicroBS(QPainter *painter, qreal scale = 1.0);
    static void drawFiberRepeater(QPainter *painter, qreal scale = 1.0);
    static void drawBBU(QPainter *painter, qreal scale = 1.0);
    static void drawRRU(QPainter *painter, qreal scale = 1.0);
    static void drawPicoStation(QPainter *painter, qreal scale = 1.0);

    // 馈线类
    static void drawFeeder12(QPainter *painter, const QPointF &start, const QPointF &end);
    static void drawFeeder78(QPainter *painter, const QPointF &start, const QPointF &end);
    static void drawJumper(QPainter *painter, const QPointF &start, const QPointF &end);
    static void drawLeakyCable(QPainter *painter, const QPointF &start, const QPointF &end);

    // 其他
    static void drawGround(QPainter *painter, qreal scale = 1.0);
    static void drawLightningProtector(QPainter *painter, qreal scale = 1.0);
    static void drawSwitch(QPainter *painter, qreal scale = 1.0);
    static void drawConnector(QPainter *painter, qreal scale = 1.0);

    // 获取器件标准尺寸
    static QRectF antennaRect(qreal scale = 1.0);
    static QRectF couplerRect(qreal scale = 1.0);
    static QRectF splitterRect(qreal scale = 1.0);
    static QRectF combinerRect(qreal scale = 1.0);
    static QRectF sourceRect(qreal scale = 1.0);

    // 获取连接点
    static QPointF couplerInput(qreal scale = 1.0);
    static QPointF couplerOutput(qreal scale = 1.0);
    static QPointF couplerCoupledPort(qreal scale = 1.0);

private:
    static void setupStandardPen(QPainter *painter, qreal width = 0.3);
    static void drawConnectionDot(QPainter *painter, const QPointF &pt);
};

} // namespace Zhifen

#endif // DEVICE_SYMBOLS_H
