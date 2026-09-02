#include "device_symbols.h"
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

namespace Zhifen {

void DeviceSymbols::setupStandardPen(QPainter *painter, qreal width, const QColor &color)
{
    QPen pen(color, width);
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
    setupStandardPen(painter, 0.35, QColor(255, 50, 50));

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
    setupStandardPen(painter, 0.35, QColor(255, 255, 0));

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
    setupStandardPen(painter, 0.35, QColor(0, 255, 255));

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


// ==================== 补充天线类 ====================

void DeviceSymbols::drawCeilingAntenna(QPainter *painter, qreal scale)
{
    // 吸顶天线：同心圆 + 安装点
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawEllipse(QPointF(0, 0), 2.5, 2.5);
    painter->drawEllipse(QPointF(0, 0), 1.5, 1.5);
    painter->drawEllipse(QPointF(0, 0), 0.5, 0.5);

    // 安装标记
    painter->drawLine(QPointF(-2.5, 0), QPointF(-3.5, 0));
    painter->drawLine(QPointF(2.5, 0), QPointF(3.5, 0));

    drawConnectionDot(painter, QPointF(0, 2.5));
    painter->restore();
}

void DeviceSymbols::drawYagiAntenna(QPainter *painter, qreal scale)
{
    // 八木天线：主梁 + 多根振子
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 主梁
    painter->drawLine(QPointF(-3, 0), QPointF(3, 0));
    // 反射器
    painter->drawLine(QPointF(-2.5, -2), QPointF(-2.5, 2));
    // 有源振子
    painter->drawLine(QPointF(-1, -1.5), QPointF(-1, 1.5));
    // 引向器
    painter->drawLine(QPointF(0.5, -1.2), QPointF(0.5, 1.2));
    painter->drawLine(QPointF(1.5, -1), QPointF(1.5, 1));
    painter->drawLine(QPointF(2.5, -0.8), QPointF(2.5, 0.8));

    drawConnectionDot(painter, QPointF(-3, 0));
    painter->restore();
}

void DeviceSymbols::drawGridAntenna(QPainter *painter, qreal scale)
{
    // 栅格天线：抛物线 + 网格
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 抛物面轮廓
    painter->drawArc(QRectF(-3, -2, 6, 4), -60 * 16, 120 * 16);
    // 网格线
    for (int i = -2; i <= 2; i++) {
        painter->drawLine(QPointF(i * 0.8, -1.5), QPointF(i * 0.8, 1.5));
    }
    // 馈源
    painter->drawLine(QPointF(0, 0), QPointF(1.5, 0));
    painter->drawEllipse(QPointF(1.5, 0), 0.5, 0.5);

    drawConnectionDot(painter, QPointF(-3, 0));
    painter->restore();
}

void DeviceSymbols::drawElevatorAntenna(QPainter *painter, qreal scale)
{
    // 电梯天线：小型平板 + 安装夹
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-1.5, -1, 3, 2));
    painter->drawLine(QPointF(-1.5, -0.5), QPointF(1.5, -0.5));
    painter->drawLine(QPointF(-1.5, 0.5), QPointF(1.5, 0.5));
    // 安装夹
    painter->drawRect(QRectF(-0.5, 1, 1, 0.8));

    drawConnectionDot(painter, QPointF(0, 1.8));
    painter->restore();
}

void DeviceSymbols::drawGPSAntenna(QPainter *painter, qreal scale)
{
    // GPS天线：蘑菇头形状
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    // 蘑菇头
    painter->drawArc(QRectF(-1.5, -2, 3, 3), 0, 180 * 16);
    painter->drawLine(QPointF(-1.5, -0.5), QPointF(1.5, -0.5));
    // 底座
    painter->drawRect(QRectF(-1, -0.5, 2, 1));
    // 引线
    painter->drawLine(QPointF(0, 0.5), QPointF(0, 2));

    drawConnectionDot(painter, QPointF(0, 2));
    painter->restore();
}

// ==================== 补充器件类 ====================

void DeviceSymbols::drawFilter(QPainter *painter, qreal scale)
{
    // 滤波器：矩形 + 内部波浪线
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-3, -1.5, 6, 3));
    // 内部滤波符号
    painter->drawLine(QPointF(-2, 0), QPointF(-1.2, -0.8));
    painter->drawLine(QPointF(-1.2, -0.8), QPointF(-0.4, 0.8));
    painter->drawLine(QPointF(-0.4, 0.8), QPointF(0.4, -0.8));
    painter->drawLine(QPointF(0.4, -0.8), QPointF(1.2, 0.8));
    painter->drawLine(QPointF(1.2, 0.8), QPointF(2, 0));

    QFont font("SimSun", 0.9);
    painter->setFont(font);
    painter->drawText(QRectF(-1, -1.3, 2, 0.8), Qt::AlignCenter, "滤波器");

    drawConnectionDot(painter, QPointF(-3, 0));
    drawConnectionDot(painter, QPointF(3, 0));
    painter->restore();
}

void DeviceSymbols::drawTrunkAmplifier(QPainter *painter, qreal scale)
{
    // 干线放大器：矩形 + 放大箭头
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-3.5, -2, 7, 4));
    // 放大符号
    painter->drawLine(QPointF(-2, 0), QPointF(1, 0));
    painter->drawLine(QPointF(1, -1), QPointF(2, 0));
    painter->drawLine(QPointF(1, 1), QPointF(2, 0));
    painter->drawLine(QPointF(2, 0), QPointF(2.5, 0));

    QFont font("SimSun", 0.9);
    painter->setFont(font);
    painter->drawText(QRectF(-2.5, -1.8, 5, 0.8), Qt::AlignCenter, "干放");

    drawConnectionDot(painter, QPointF(-3.5, 0));
    drawConnectionDot(painter, QPointF(3.5, 0));
    painter->restore();
}

void DeviceSymbols::drawPOI(QPainter *painter, qreal scale)
{
    // POI多系统合路平台：大矩形 + 多输入单输出
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-4, -3, 8, 6));
    // 多输入（左侧4个）
    for (int i = 0; i < 4; i++) {
        qreal y = -2.25 + i * 1.5;
        painter->drawLine(QPointF(-5.5, y), QPointF(-4, y));
        drawConnectionDot(painter, QPointF(-5.5, y));
    }
    // 单输出（右侧）
    painter->drawLine(QPointF(4, 0), QPointF(5.5, 0));
    drawConnectionDot(painter, QPointF(5.5, 0));

    QFont font("SimSun", 1.2);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.8, 4, 1.6), Qt::AlignCenter, "POI");

    painter->restore();
}

void DeviceSymbols::drawCavitySplitter(QPainter *painter, qreal scale, int ways)
{
    // 腔体功分器：矩形 + 内部腔体标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-3, -2, 6, 4));
    // 内部腔体分隔
    painter->drawLine(QPointF(-1, -2), QPointF(-1, 2));
    painter->drawLine(QPointF(1, -2), QPointF(1, 2));

    QFont font("SimSun", 0.9);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.5, 4, 1), Qt::AlignCenter, QString("腔体%1分").arg(ways));

    drawConnectionDot(painter, QPointF(-3, 0));
    if (ways == 2) {
        drawConnectionDot(painter, QPointF(3, -1));
        drawConnectionDot(painter, QPointF(3, 1));
    } else if (ways == 3) {
        drawConnectionDot(painter, QPointF(3, -1.5));
        drawConnectionDot(painter, QPointF(3, 0));
        drawConnectionDot(painter, QPointF(3, 1.5));
    } else {
        drawConnectionDot(painter, QPointF(3, -1.8));
        drawConnectionDot(painter, QPointF(3, -0.6));
        drawConnectionDot(painter, QPointF(3, 0.6));
        drawConnectionDot(painter, QPointF(3, 1.8));
    }
    painter->restore();
}

// ==================== 数字化室分器件 ====================

void DeviceSymbols::drawMAU(QPainter *painter, qreal scale)
{
    // 主控单元MAU：大矩形 + 多端口
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-5, -3, 10, 6));
    // 端口标识
    for (int i = 0; i < 4; i++) {
        painter->drawRect(QRectF(-4 + i * 2.2, 2, 1.2, 0.8));
    }

    QFont font("SimSun", 1.5);
    painter->setFont(font);
    painter->drawText(QRectF(-3, -1.2, 6, 2.4), Qt::AlignCenter, "MAU\n主控单元");

    drawConnectionDot(painter, QPointF(0, 3));
    painter->restore();
}

void DeviceSymbols::drawEU(QPainter *painter, qreal scale)
{
    // 扩展单元EU：中等矩形
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-4, -2.5, 8, 5));
    for (int i = 0; i < 3; i++) {
        painter->drawRect(QRectF(-3 + i * 2.5, 1.8, 1.2, 0.7));
    }

    QFont font("SimSun", 1.3);
    painter->setFont(font);
    painter->drawText(QRectF(-2.5, -1, 5, 2), Qt::AlignCenter, "EU\n扩展单元");

    drawConnectionDot(painter, QPointF(0, 2.5));
    painter->restore();
}

void DeviceSymbols::drawPRRU(QPainter *painter, qreal scale)
{
    // 远端单元pRRU：小矩形 + 天线标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-2.5, -2, 5, 4));
    // 天线标识
    painter->drawLine(QPointF(0, -2), QPointF(0, -3));
    painter->drawLine(QPointF(-0.8, -2.8), QPointF(0, -3));
    painter->drawLine(QPointF(0.8, -2.8), QPointF(0, -3));

    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.8, 4, 1.6), Qt::AlignCenter, "pRRU");

    drawConnectionDot(painter, QPointF(0, 2));
    painter->restore();
}

void DeviceSymbols::drawPOESwitch(QPainter *painter, qreal scale)
{
    // POE交换机：矩形 + 多网口
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-5, -1.5, 10, 3));
    // 网口
    for (int i = 0; i < 8; i++) {
        painter->drawRect(QRectF(-4.2 + i * 1.1, -0.5, 0.8, 1));
    }

    QFont font("SimSun", 0.8);
    painter->setFont(font);
    painter->drawText(QRectF(-3, -1.4, 6, 0.7), Qt::AlignCenter, "POE交换机");

    drawConnectionDot(painter, QPointF(-5, 0));
    drawConnectionDot(painter, QPointF(5, 0));
    painter->restore();
}

void DeviceSymbols::drawOpticalModule(QPainter *painter, qreal scale)
{
    // 光模块：小矩形 + 光纤接口
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-1.5, -0.8, 3, 1.6));
    painter->drawRect(QRectF(1.5, -0.5, 0.8, 1));

    QFont font("SimSun", 0.6);
    painter->setFont(font);
    painter->drawText(QRectF(-1.2, -0.4, 2.4, 0.8), Qt::AlignCenter, "光模块");

    drawConnectionDot(painter, QPointF(-1.5, 0));
    painter->restore();
}

// ==================== 补充馈线类 ====================

void DeviceSymbols::drawFeeder158(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 1-5/8馈线：更粗实线
    painter->save();
    QPen pen(Qt::black, 0.9);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawSuperFlexJumper(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 超柔跳线：点虚线
    painter->save();
    QPen pen(Qt::black, 0.2, Qt::DotLine);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawOpticalFiber(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 光纤：蓝色细线
    painter->save();
    QPen pen(QColor(0, 100, 255), 0.2);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

void DeviceSymbols::drawNetworkCable(QPainter *painter, const QPointF &start, const QPointF &end)
{
    // 网线：绿色双线
    painter->save();
    QPen pen(QColor(0, 150, 0), 0.25);
    painter->setPen(pen);
    painter->drawLine(start, end);
    qreal dx = end.x() - start.x();
    qreal dy = end.y() - start.y();
    qreal len = qSqrt(dx*dx + dy*dy);
    if (len > 0) {
        qreal nx = -dy / len * 0.3;
        qreal ny = dx / len * 0.3;
        painter->drawLine(start + QPointF(nx, ny), end + QPointF(nx, ny));
    }
    painter->restore();
}

// ==================== 补充辅助器件 ====================

void DeviceSymbols::drawGroundingKit(QPainter *painter, qreal scale)
{
    // 接地卡：卡箍 + 接地线
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawArc(QRectF(-1.5, -1, 3, 2), 0, 180 * 16);
    painter->drawLine(QPointF(-1.5, 0), QPointF(1.5, 0));
    painter->drawLine(QPointF(0, 0), QPointF(0, 2));
    painter->drawLine(QPointF(-1, 2), QPointF(1, 2));
    painter->drawLine(QPointF(-0.6, 2.6), QPointF(0.6, 2.6));

    drawConnectionDot(painter, QPointF(0, 2.6));
    painter->restore();
}

void DeviceSymbols::drawSurgeProtector(QPainter *painter, qreal scale)
{
    // 浪涌保护器：矩形 + 保护符号
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);

    painter->drawRect(QRectF(-2, -1.5, 4, 3));
    // 保护符号（三角形+箭头）
    painter->drawLine(QPointF(-1, -1), QPointF(0, 1));
    painter->drawLine(QPointF(0, 1), QPointF(1, -1));
    painter->drawLine(QPointF(-1, -1), QPointF(1, -1));

    QFont font("SimSun", 0.7);
    painter->setFont(font);
    painter->drawText(QRectF(-1.5, -1.3, 3, 0.6), Qt::AlignCenter, "浪涌");

    drawConnectionDot(painter, QPointF(-2, 0));
    drawConnectionDot(painter, QPointF(2, 0));
    painter->restore();
}

void DeviceSymbols::drawNConnector(QPainter *painter, qreal scale)
{
    // N型接头：圆形 + 螺纹标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.3);

    painter->drawEllipse(QPointF(0, 0), 1, 1);
    painter->drawEllipse(QPointF(0, 0), 0.5, 0.5);
    // 螺纹
    painter->drawArc(QRectF(-0.8, -0.8, 1.6, 1.6), 45 * 16, 90 * 16);
    painter->drawArc(QRectF(-0.8, -0.8, 1.6, 1.6), 225 * 16, 90 * 16);

    painter->restore();
}

void DeviceSymbols::drawDINConnector(QPainter *painter, qreal scale)
{
    // DIN型接头：大圆形 + 螺纹标识
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.3);

    painter->drawEllipse(QPointF(0, 0), 1.3, 1.3);
    painter->drawEllipse(QPointF(0, 0), 0.7, 0.7);
    painter->drawEllipse(QPointF(0, 0), 0.3, 0.3);
    // 螺纹
    for (int i = 0; i < 4; i++) {
        painter->drawArc(QRectF(-1.1, -1.1, 2.2, 2.2), (i * 90 + 30) * 16, 30 * 16);
    }

    painter->restore();
}

// ==================== 天越11大类补充器件 ====================

// 主接点（天越特有：馈线主干连接点）
void DeviceSymbols::drawMainJunction(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.4);
    // 实心圆点
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(QPointF(0, 0), 1.0, 1.0);
    painter->restore();
}

// 副接点（天越特有：分支连接点）
void DeviceSymbols::drawSubJunction(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    // 空心圆点
    painter->drawEllipse(QPointF(0, 0), 0.8, 0.8);
    painter->restore();
}

// 固定衰减器
void DeviceSymbols::drawFixedAttenuator(QPainter *painter, qreal scale, int db)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    // 矩形 + 衰减标识
    painter->drawRect(QRectF(-2.5, -1, 5, 2));
    QFont font("SimSun", 0.8);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.6, 4, 1.2), Qt::AlignCenter, QString("%1dB").arg(db));
    drawConnectionDot(painter, QPointF(-2.5, 0));
    drawConnectionDot(painter, QPointF(2.5, 0));
    painter->restore();
}

// 可调衰减器
void DeviceSymbols::drawVariableAttenuator(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    painter->drawRect(QRectF(-2.5, -1, 5, 2));
    // 可调箭头
    painter->drawLine(QPointF(-1.5, -1.5), QPointF(1.5, 1.5));
    painter->drawLine(QPointF(1.0, 1.5), QPointF(1.5, 1.5));
    painter->drawLine(QPointF(1.5, 1.0), QPointF(1.5, 1.5));
    QFont font("SimSun", 0.7);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -0.8, 4, 0.6), Qt::AlignCenter, "可调");
    drawConnectionDot(painter, QPointF(-2.5, 0));
    drawConnectionDot(painter, QPointF(2.5, 0));
    painter->restore();
}

// 光端机
void DeviceSymbols::drawOpticalTerminal(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    painter->drawRect(QRectF(-3, -2, 6, 4));
    QFont font("SimSun", 1.0);
    painter->setFont(font);
    painter->drawText(QRectF(-2.5, -0.8, 5, 1.6), Qt::AlignCenter, "光端机");
    // 光口标识
    painter->drawEllipse(QPointF(-2, 1.2), 0.4, 0.4);
    painter->drawEllipse(QPointF(2, 1.2), 0.4, 0.4);
    drawConnectionDot(painter, QPointF(0, 2));
    painter->restore();
}

// 干放（干线放大器，天越常用名称）
void DeviceSymbols::drawRepeater(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    painter->drawRect(QRectF(-3, -1.8, 6, 3.6));
    // 放大符号
    painter->drawLine(QPointF(-1.8, 0), QPointF(0.8, 0));
    painter->drawLine(QPointF(0.8, -0.9), QPointF(1.8, 0));
    painter->drawLine(QPointF(0.8, 0.9), QPointF(1.8, 0));
    QFont font("SimSun", 0.9);
    painter->setFont(font);
    painter->drawText(QRectF(-2, -1.6, 4, 0.8), Qt::AlignCenter, "干放");
    drawConnectionDot(painter, QPointF(-3, 0));
    drawConnectionDot(painter, QPointF(3, 0));
    painter->restore();
}

// 光纤
void DeviceSymbols::drawFiberCable(QPainter *painter, const QPointF &start, const QPointF &end)
{
    painter->save();
    QPen pen(QColor(255, 0, 0), 0.25, Qt::DashLine);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

// 网线（CAT6）
void DeviceSymbols::drawLANCable(QPainter *painter, const QPointF &start, const QPointF &end)
{
    painter->save();
    QPen pen(QColor(0, 128, 0), 0.3);
    painter->setPen(pen);
    painter->drawLine(start, end);
    painter->restore();
}

// GPS天线（蘑菇头，天越常用）
void DeviceSymbols::drawGPSMushroom(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    // 蘑菇头
    painter->drawArc(QRectF(-1.2, -1.8, 2.4, 2.4), 0, 180 * 16);
    painter->drawLine(QPointF(-1.2, -0.6), QPointF(1.2, -0.6));
    // 底座
    painter->drawRect(QRectF(-0.8, -0.6, 1.6, 0.8));
    // 引线
    painter->drawLine(QPointF(0, 0.2), QPointF(0, 1.8));
    drawConnectionDot(painter, QPointF(0, 1.8));
    painter->restore();
}

// 对数周期天线
void DeviceSymbols::drawLogPeriodicAntenna(QPainter *painter, qreal scale)
{
    painter->save();
    painter->scale(scale, scale);
    setupStandardPen(painter, 0.35);
    // 主梁
    painter->drawLine(QPointF(-3, 0), QPointF(3, 0));
    // 多组振子（对数周期，长度递增）
    for (int i = 0; i < 5; i++) {
        qreal x = -2 + i * 1.0;
        qreal len = 0.5 + i * 0.4;
        painter->drawLine(QPointF(x, -len), QPointF(x, len));
    }
    drawConnectionDot(painter, QPointF(-3, 0));
    painter->restore();
}

} // namespace Zhifen
