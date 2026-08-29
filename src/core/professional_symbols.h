#ifndef PROFESSIONAL_SYMBOLS_H
#define PROFESSIONAL_SYMBOLS_H

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QPainter>
#include <QRectF>
#include <QPointF>
#include <QPen>
#include <QBrush>
#include <QString>

namespace Zhifen {

// 专业器件符号绘制器
class ProfessionalSymbolPainter
{
public:
    // 绘制信源（基站/直放站）
    static QGraphicsItemGroup* drawSource(QGraphicsItem *parent = nullptr);
    // 绘制全向吸顶天线
    static QGraphicsItemGroup* drawOmniCeilingAntenna(QGraphicsItem *parent = nullptr);
    // 绘制定向板状天线
    static QGraphicsItemGroup* drawPanelAntenna(QGraphicsItem *parent = nullptr);
    // 绘制壁挂天线
    static QGraphicsItemGroup* drawWallAntenna(QGraphicsItem *parent = nullptr);
    // 绘制功分器（二功分/三功分/四功分）
    static QGraphicsItemGroup* drawPowerSplitter(int ways, QGraphicsItem *parent = nullptr);
    // 绘制耦合器（5/6/7/10/12/15/20/30/40dB）
    static QGraphicsItemGroup* drawCoupler(qreal db, QGraphicsItem *parent = nullptr);
    // 绘制合路器
    static QGraphicsItemGroup* drawCombiner(int ways, QGraphicsItem *parent = nullptr);
    // 绘制负载
    static QGraphicsItemGroup* drawLoad(QGraphicsItem *parent = nullptr);
    // 绘制衰减器
    static QGraphicsItemGroup* drawAttenuator(qreal db, QGraphicsItem *parent = nullptr);
    // 绘制光纤天线
    static QGraphicsItemGroup* drawFiberAntenna(QGraphicsItem *parent = nullptr);
    // 绘制漏缆
    static QGraphicsItemGroup* drawLeakyCable(QGraphicsItem *parent = nullptr);
    // 绘制射灯天线（楼间对打）
    static QGraphicsItemGroup* drawSpotlightAntenna(QGraphicsItem *parent = nullptr);
    // 绘制外引天线
    static QGraphicsItemGroup* drawExternalAntenna(QGraphicsItem *parent = nullptr);
    // 绘制5G pRRU（数字化室分）
    static QGraphicsItemGroup* drawPRRU(QGraphicsItem *parent = nullptr);
    // 绘制5G RHUB（数字化室分）
    static QGraphicsItemGroup* drawRHUB(QGraphicsItem *parent = nullptr);
    // 绘制5G BBU（数字化室分）
    static QGraphicsItemGroup* drawBBU(QGraphicsItem *parent = nullptr);

    // 根据器件型号自动绘制对应符号
    static QGraphicsItemGroup* drawByModel(const QString &model, QGraphicsItem *parent = nullptr);

    // 添加型号标签
    static void addModelLabel(QGraphicsItemGroup *group, const QString &model, const QPointF &pos);

private:
    // 标准画笔
    static QPen standardPen();
    static QPen thickPen();
    static QBrush standardBrush();
    // 绘制端口
    static void drawPort(QGraphicsItemGroup *group, const QPointF &pos, const QString &label = "");
    // 绘制箭头
    static void drawArrow(QGraphicsItemGroup *group, const QPointF &start, const QPointF &end);
};

} // namespace Zhifen

#endif // PROFESSIONAL_SYMBOLS_H
