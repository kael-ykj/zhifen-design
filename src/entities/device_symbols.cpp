#include "device_symbols.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

namespace Zhifen {

void DeviceSymbols::setupStandardPen(QPainter *painter, qreal width)
{
    QPen pen(Qt::black, width);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
}

void DeviceSymbols::drawConnectionDot(QPainter *painter, const QPointF &pt)
{
    painter->save();
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(pt, 0.4, 0.4);
    painter->restore();
}

// ==================== 天线类 ====================

void DeviceSymbols::drawOmniAntenna(QPainter *painter, qreal scale)
{
    // 全向天线：圆 + 内部辐射箭头（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 外圆
    painter->drawEllipse(QPointF(0, 0), 2.5, 2.5);

    // 内部辐射图案：从中心向外的箭头
    painter->drawLine(QPointF(0, 0), QPointF(0, -1.8));
    painter->drawLine(QPointF(0, 0), QPointF(1.27, -1.27));
    painter->drawLine(QPointF(0, 0), QPointF(1.8, 0));
    painter->drawLine(QPointF(0, 0), QPointF(1.27, 1.27));
    painter->drawLine(QPointF(0, 0), QPointF(0, 1.8));
    painter->drawLine(QPointF(0, 0), QPointF(-1.27, 1.27));
    painter->drawLine(QPointF(0, 0), QPointF(-1.8, 0));
    painter->drawLine(QPointF(0, 0), QPointF(-1.27, -1.27));

    // 中心点
    drawConnectionDot(painter, QPointF(0, 0));

    painter->restore();
}

void DeviceSymbols::drawDirectionalAntenna(QPainter *painter, qreal scale)
{
    // 定向天线：三角形 + 底边（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 三角形（指向右）
    QPolygonF triangle;
    triangle << QPointF(-2, -2) << QPointF(2, 0) << QPointF(-2, 2);
    painter->drawPolygon(triangle);

    // 底边
    painter->drawLine(QPointF(-2, -2), QPointF(-2, 2));

    // 连接点在左侧中心
    drawConnectionDot(painter, QPointF(-2, 0));

    painter->restore();
}

void DeviceSymbols::drawSpotlightAntenna(QPainter *painter, qreal scale)
{
    // 射灯天线：圆形 + 灯座（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 灯头圆
    painter->drawEllipse(QPointF(0, 0), 2, 2);

    // 内部同心圆
    painter->drawEllipse(QPointF(0, 0), 1.2, 1.2);

    // 灯座（矩形）
    painter->drawRect(QRectF(-1, -3, 2, 1));

    // 安装底座
    painter->drawLine(QPointF(-1.5, -3), QPointF(1.5, -3));

    // 连接点
    drawConnectionDot(painter, QPointF(0, -3));

    painter->restore();
}

void DeviceSymbols::drawExternalAntenna(QPainter *painter, qreal scale)
{
    // 外引天线：菱形 + 引线（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 菱形
    QPolygonF diamond;
    diamond << QPointF(0, -2.5) << QPointF(2, 0) << QPointF(0, 2.5) << QPointF(-2, 0);
    painter->drawPolygon(diamond);

    // 内部交叉线
    painter->drawLine(QPointF(0, -2.5), QPointF(0, 2.5));
    painter->drawLine(QPointF(-2, 0), QPointF(2, 0));

    // 引线
    painter->drawLine(QPointF(0, 2.5), QPointF(0, 4));

    // 连接点
    drawConnectionDot(painter, QPointF(0, 4));

    painter->restore();
}

void DeviceSymbols::drawWallMountAntenna(QPainter *painter, qreal scale)
{
    // 壁挂天线：矩形 + 安装边
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 矩形主体
    painter->drawRect(QRectF(-2, -1.5, 4, 3));

    // 安装边（左侧）
    painter->drawLine(QPointF(-2.5, -2), QPointF(-2.5, 2));
    painter->drawLine(QPointF(-2.5, -2), QPointF(-2, -1.5));
    painter->drawLine(QPointF(-2.5, 2), QPointF(-2, 1.5));

    // 连接点
    drawConnectionDot(painter, QPointF(-2.5, 0));

    painter->restore();
}

// ==================== 器件类 ====================

void DeviceSymbols::drawCoupler(QPainter *painter, qreal scale, int db)
{
    // 耦合器：矩形 + 直通端 + 耦合端（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 主体矩形
    painter->drawRect(QRectF(-4, -1.5, 8, 3));

    // 内部耦合线
    painter->drawLine(QPointF(-3, 0.5), QPointF(3, 0.5));

    // 耦合端箭头（向下）
    painter->drawLine(QPointF(0, 0.5), QPointF(0, 2.5));
    painter->drawLine(QPointF(-0.5, 2), QPointF(0, 2.5));
    painter->drawLine(QPointF(0.5, 2), QPointF(0, 2.5));

    // 标注dB值
    QFont font("SimSun", 1.2);
    painter->setFont(font);
    painter->drawText(QRectF(-1, -1.2, 2, 1), Qt::AlignCenter, QString("%1dB").arg(db));

    // 连接点
    drawConnectionDot(painter, QPointF(-4, 0));   // 输入端
    drawConnectionDot(painter, QPointF(4, 0));    // 输出端
    drawConnectionDot(painter, QPointF(0, 2.5));  // 耦合端

    painter->restore();
}

void DeviceSymbols::drawSplitter(QPainter *painter, qreal scale, int ways)
{
    // 功分器：Y形 / 树形（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    if (ways == 2) {
        // 二功分：Y形
        painter->drawLine(QPointF(-4, 0), QPointF(-1, 0));
        painter->drawLine(QPointF(-1, 0), QPointF(2, -2));
        painter->drawLine(QPointF(-1, 0), QPointF(2, 2));

        // 连接点
        drawConnectionDot(painter, QPointF(-4, 0));
        drawConnectionDot(painter, QPointF(2, -2));
        drawConnectionDot(painter, QPointF(2, 2));
    } else if (ways == 3) {
        // 三功分
        painter->drawLine(QPointF(-4, 0), QPointF(-1, 0));
        painter->drawLine(QPointF(-1, 0), QPointF(2, -2.5));
        painter->drawLine(QPointF(-1, 0), QPointF(2, 0));
        painter->drawLine(QPointF(-1, 0), QPointF(2, 2.5));

        drawConnectionDot(painter, QPointF(-4, 0));
        drawConnectionDot(painter, QPointF(2, -2.5));
        drawConnectionDot(painter, QPointF(2, 0));
        drawConnectionDot(painter, QPointF(2, 2.5));
    } else {
        // 四功分
        painter->drawLine(QPointF(-4, 0), QPointF(-1, 0));
        painter->drawLine(QPointF(-1, 0), QPointF(2, -3));
        painter->drawLine(QPointF(-1, 0), QPointF(2, -1));
        painter->drawLine(QPointF(-1, 0), QPointF(2, 1));
        painter->drawLine(QPointF(-1, 0), QPointF(2, 3));

        drawConnectionDot(painter, QPointF(-4, 0));
        drawConnectionDot(painter, QPointF(2, -3));
        drawConnectionDot(painter, QPointF(2, -1));
        drawConnectionDot(painter, QPointF(2, 1));
        drawConnectionDot(painter, QPointF(2, 3));
    }

    // 标注
    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-0.5, -0.5, 1, 1), Qt::AlignCenter, QString("%1").arg(ways));

    painter->restore();
}

void DeviceSymbols::drawCombiner(QPainter *painter, qreal scale)
{
    // 合路器：两个输入合并为一个输出（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 主体菱形
    QPolygonF diamond;
    diamond << QPointF(-3, 0) << QPointF(0, -2.5) << QPointF(3, 0) << QPointF(0, 2.5);
    painter->drawPolygon(diamond);

    // 输入线（左侧两个）
    painter->drawLine(QPointF(-5, -1.5), QPointF(-3, 0));
    painter->drawLine(QPointF(-5, 1.5), QPointF(-3, 0));

    // 输出线（右侧一个）
    painter->drawLine(QPointF(3, 0), QPointF(5, 0));

    // 连接点
    drawConnectionDot(painter, QPointF(-5, -1.5));
    drawConnectionDot(painter, QPointF(-5, 1.5));
    drawConnectionDot(painter, QPointF(5, 0));

    painter->restore();
}

void DeviceSymbols::drawHybrid(QPainter *painter, qreal scale)
{
    // 电桥：3dB电桥，方形 + 内部交叉
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 主体方形
    painter->drawRect(QRectF(-3, -3, 6, 6));

    // 内部交叉线
    painter->drawLine(QPointF(-3, -3), QPointF(3, 3));
    painter->drawLine(QPointF(-3, 3), QPointF(3, -3));

    // 四个端口
    drawConnectionDot(painter, QPointF(-3, -1.5));
    drawConnectionDot(painter, QPointF(-3, 1.5));
    drawConnectionDot(painter, QPointF(3, -1.5));
    drawConnectionDot(painter, QPointF(3, 1.5));

    // 标注
    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-1.5, -0.5, 3, 1), Qt::AlignCenter, "3dB");

    painter->restore();
}

void DeviceSymbols::drawAttenuator(QPainter *painter, qreal scale, int db)
{
    // 衰减器：锯齿形（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 锯齿形
    QPolygonF sawtooth;
    sawtooth << QPointF(-4, 0) << QPointF(-3, -1.5) << QPointF(-2, 1.5)
             << QPointF(-1, -1.5) << QPointF(0, 1.5) << QPointF(1, -1.5)
             << QPointF(2, 1.5) << QPointF(3, -1.5) << QPointF(4, 0);
    painter->drawPolyline(sawtooth);

    // 标注
    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-1, -2.5, 2, 1), Qt::AlignCenter, QString("%1dB").arg(db));

    // 连接点
    drawConnectionDot(painter, QPointF(-4, 0));
    drawConnectionDot(painter, QPointF(4, 0));

    painter->restore();
}

void DeviceSymbols::drawLoad(QPainter *painter, qreal scale)
{
    // 负载：矩形 + 斜线（行业标准画法）
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 矩形
    painter->drawRect(QRectF(-2, -2, 4, 4));

    // 内部斜线
    painter->drawLine(QPointF(-2, -2), QPointF(2, 2));
    painter->drawLine(QPointF(-2, 2), QPointF(2, -2));

    // 连接点
    drawConnectionDot(painter, QPointF(-2, 0));

    painter->restore();
}

void DeviceSymbols::drawIsolator(QPainter *painter, qreal scale)
{
    // 隔离器：箭头 + 矩形
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 矩形
    painter->drawRect(QRectF(-2.5, -1.5, 5, 3));

    // 箭头（单向）
    painter->drawLine(QPointF(-1.5, 0), QPointF(1.5, 0));
    painter->drawLine(QPointF(0.8, -0.6), QPointF(1.5, 0));
    painter->drawLine(QPointF(0.8, 0.6), QPointF(1.5, 0));

    // 连接点
    drawConnectionDot(painter, QPointF(-2.5, 0));
    drawConnectionDot(painter, QPointF(2.5, 0));

    painter->restore();
}

void DeviceSymbols::drawCirculator(QPainter *painter, qreal scale)
{
    // 环形器：圆形 + 循环箭头
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 圆形
    painter->drawEllipse(QPointF(0, 0), 2.5, 2.5);

    // 循环箭头（顺时针）
    QRectF arcRect(-1.5, -1.5, 3, 3);
    painter->drawArc(arcRect, 30 * 16, 270 * 16);
    // 箭头
    painter->drawLine(QPointF(1.0, -0.8), QPointF(1.3, -0.3));
    painter->drawLine(QPointF(0.5, -0.5), QPointF(1.3, -0.3));

    // 三个端口
    drawConnectionDot(painter, QPointF(0, -2.5));
    drawConnectionDot(painter, QPointF(2.17, 1.25));
    drawConnectionDot(painter, QPointF(-2.17, 1.25));

    painter->restore();
}

// ==================== 信源类 ====================

void DeviceSymbols::drawMacroBS(QPainter *painter, qreal scale)
{
    // 宏基站：大矩形 + 铁塔标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 矩形
    painter->drawRect(QRectF(-5, -3, 10, 6));

    // 内部文字
    QFont font("SimSun", 1.5);
    painter->setFont(font);
    painter->drawText(QRectF(-4, -1, 8, 2), Qt::AlignCenter, "宏基站");

    // 铁塔标识（顶部）
    painter->drawLine(QPointF(-1, -3), QPointF(-1, -5));
    painter->drawLine(QPointF(1, -3), QPointF(1, -5));
    painter->drawLine(QPointF(-1.5, -5), QPointF(1.5, -5));

    // 连接点
    drawConnectionDot(painter, QPointF(0, 3));

    painter->restore();
}

void DeviceSymbols::drawMicroBS(QPainter *painter, qreal scale)
{
    // 微基站：中等矩形
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-4, -2.5, 8, 5));

    QFont font("SimSun", 1.3);
    painter->setFont(font);
    painter->drawText(QRectF(-3, -0.8, 6, 1.6), Qt::AlignCenter, "微基站");

    drawConnectionDot(painter, QPointF(0, 2.5));

    painter->restore();
}

void DeviceSymbols::drawFiberRepeater(QPainter *painter, qreal scale)
{
    // 光纤直放站：矩形 + 光纤标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-4, -2.5, 8, 5));

    QFont font("SimSun", 1.1);
    painter->setFont(font);
    painter->drawText(QRectF(-3.5, -0.7, 7, 1.4), Qt::AlignCenter, "光纤直放站");

    // 光纤标识（波浪线）
    painter->drawLine(QPointF(-3, 1.5), QPointF(-2, 1.2));
    painter->drawLine(QPointF(-2, 1.2), QPointF(-1, 1.8));
    painter->drawLine(QPointF(-1, 1.8), QPointF(0, 1.2));
    painter->drawLine(QPointF(0, 1.2), QPointF(1, 1.8));
    painter->drawLine(QPointF(1, 1.8), QPointF(2, 1.2));
    painter->drawLine(QPointF(2, 1.2), QPointF(3, 1.5));

    drawConnectionDot(painter, QPointF(0, 2.5));

    painter->restore();
}

void DeviceSymbols::drawBBU(QPainter *painter, qreal scale)
{
    // BBU：基带处理单元
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-3.5, -2, 7, 4));

    QFont font("SimSun", 1.2);
    painter->setFont(font);
    painter->drawText(QRectF(-3, -0.6, 6, 1.2), Qt::AlignCenter, "BBU");

    drawConnectionDot(painter, QPointF(0, 2));

    painter->restore();
}

void DeviceSymbols::drawRRU(QPainter *painter, qreal scale)
{
    // RRU：射频拉远单元
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-2.5, -2, 5, 4));

    QFont font("SimSun", 1.1);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.5, 4, 1), Qt::AlignCenter, "RRU");

    drawConnectionDot(painter, QPointF(0, 2));

    painter->restore();
}

void DeviceSymbols::drawPicoStation(QPainter *painter, qreal scale)
{
    // 皮基站：小矩形
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-2.5, -2, 5, 4));

    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.5, 4, 1), Qt::AlignCenter, "pRRU");

    drawConnectionDot(painter, QPointF(0, 2));

    painter->restore();
}

// ==================== 馈线类 ====================

void DeviceSymbols::drawFeeder12(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 1/2馈线：细实线
    painter->save();
    QPen pen(Qt::black, 0.3);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawFeeder78(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 7/8馈线：粗实线
    painter->save();
    QPen pen(Qt::black, 0.6);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawJumper(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 跳线：虚线
    painter->save();
    QPen pen(Qt::black, 0.25, Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawLeakyCable(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 漏缆：点划线 + 辐射标记
    painter->save();
    QPen pen(Qt::black, 0.4, Qt::DashDotLine);
    painter->setPen(pen);
    painter->drawLine(start, end);

    // 辐射标记（每隔一段画一个小箭头）
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    qreal len = qSqrt(dx*dx + dy*dy);
    int segments = qMax(1, (int)(len / 5));
    for (int i = 1; i < segments; i++) {
        QPointF pt(start.x() + dx * i / segments, start.y() + dy * i / segments);
        painter->drawLine(pt, QPointF(pt.x() + 1, pt.y() + 1));
        painter->drawLine(pt, QPointF(pt.x() - 1, pt.y() - 1));
    }

    painter->restore();
}

// ==================== 其他 ====================

void DeviceSymbols::drawGround(QPainter *painter, qreal scale)
{
    // 接地：三条递减横线
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawLine(QPointF(0, 0), QPointF(0, 1));
    painter->drawLine(QPointF(-2, 1), QPointF(2, 1));
    painter->drawLine(QPointF(-1.3, 1.8), QPointF(1.3, 1.8));
    painter->drawLine(QPointF(-0.6, 2.6), QPointF(0.6, 2.6));

    drawConnectionDot(painter, QPointF(0, 0));

    painter->restore();
}

void DeviceSymbols::drawLightningProtector(QPainter *painter, qreal scale)
{
    // 防雷器：矩形 + 闪电符号
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-2, -2, 4, 4));

    // 闪电符号
    QPolygonF lightning;
    lightning << QPointF(0.3, -1.2) << QPointF(-0.5, 0.2) << QPointF(0.2, 0.2)
              << QPointF(-0.3, 1.2);
    painter->drawPolyline(lightning);

    drawConnectionDot(painter, QPointF(-2, 0));
    drawConnectionDot(painter, QPointF(2, 0));

    painter->restore();
}

void DeviceSymbols::drawSwitch(QPainter *painter, qreal scale)
{
    // 开关：断开的线 + 旋转臂
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawLine(QPointF(-3, 0), QPointF(-1, 0));
    painter->drawLine(QPointF(1, 0), QPointF(3, 0));
    painter->drawLine(QPointF(-1, 0), QPointF(1, 1.5));

    drawConnectionDot(painter, QPointF(-3, 0));
    drawConnectionDot(painter, QPointF(3, 0));

    painter->restore();
}

void DeviceSymbols::drawConnector(QPainter *painter, qreal scale)
{
    // 接头：小圆圈
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.3);

    painter->drawEllipse(QPointF(0, 0), 0.8, 0.8);
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(QPointF(0, 0), 0.3, 0.3);

    painter->restore();
}

// ==================== 尺寸和连接点 ====================

QRectF DeviceSymbols::antennaRect(qreal scale)
{
    return QRectF(-3 * scale, -3 * scale, 6 * scale, 6 * scale);
}

QRectF DeviceSymbols::couplerRect(qreal scale)
{
    return QRectF(-4.5 * scale, -2 * scale, 9 * scale, 5 * scale);
}

QRectF DeviceSymbols::splitterRect(qreal scale)
{
    return QRectF(-4.5 * scale, -3.5 * scale, 7 * scale, 7 * scale);
}

QRectF DeviceSymbols::combinerRect(qreal scale)
{
    return QRectF(-5.5 * scale, -3 * scale, 11 * scale, 6 * scale);
}

QRectF DeviceSymbols::sourceRect(qreal scale)
{
    return QRectF(-5.5 * scale, -5 * scale, 11 * scale, 10 * scale);
}

QPointF DeviceSymbols::couplerInput(qreal scale)
{
    return QPointF(-4 * scale, 0);
}

QPointF DeviceSymbols::couplerOutput(qreal scale)
{
    return QPointF(4 * scale, 0);
}

QPointF DeviceSymbols::couplerCoupledPort(qreal scale)
{
    return QPointF(0, 2.5 * scale);
}

} // namespace Zhifen
