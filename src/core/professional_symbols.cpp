#include "professional_symbols.h"
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPolygonF>
#include <QtMath>

namespace Zhifen {

QPen ProfessionalSymbolPainter::standardPen() {
    return QPen(QColor(0, 0, 0), 0.5, Qt::SolidLine);
}

QPen ProfessionalSymbolPainter::thickPen() {
    return QPen(QColor(0, 0, 0), 1.0, Qt::SolidLine);
}

QBrush ProfessionalSymbolPainter::standardBrush() {
    return QBrush(Qt::NoBrush);
}

void ProfessionalSymbolPainter::drawPort(QGraphicsItemGroup *group, const QPointF &pos, const QString &label) {
    QGraphicsEllipseItem *port = new QGraphicsEllipseItem(pos.x() - 1, pos.y() - 1, 2, 2, group);
    port->setBrush(QBrush(QColor(0, 0, 0)));
    port->setPen(Qt::NoPen);
    if (!label.isEmpty()) {
        QGraphicsTextItem *text = new QGraphicsTextItem(label, group);
        text->setFont(QFont("SimSun", 4));
        text->setPos(pos.x() + 2, pos.y() - 4);
    }
}

void ProfessionalSymbolPainter::drawArrow(QGraphicsItemGroup *group, const QPointF &start, const QPointF &end) {
    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(start, end), group);
    line->setPen(standardPen());

    qreal angle = qAtan2(end.y() - start.y(), end.x() - start.x());
    qreal arrowLen = 2;
    QPointF p1(end.x() - arrowLen * qCos(angle - M_PI / 6),
               end.y() - arrowLen * qSin(angle - M_PI / 6));
    QPointF p2(end.x() - arrowLen * qCos(angle + M_PI / 6),
               end.y() - arrowLen * qSin(angle + M_PI / 6));
    QPolygonF arrow;
    arrow << end << p1 << p2;
    QGraphicsPolygonItem *arrowItem = new QGraphicsPolygonItem(arrow, group);
    arrowItem->setBrush(QBrush(QColor(0, 0, 0)));
    arrowItem->setPen(Qt::NoPen);
}

void ProfessionalSymbolPainter::addModelLabel(QGraphicsItemGroup *group, const QString &model, const QPointF &pos) {
    QGraphicsTextItem *text = new QGraphicsTextItem(model, group);
    text->setFont(QFont("SimSun", 5));
    text->setPos(pos);
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawSource(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 信源：矩形+基站符号
    QGraphicsRectItem *rect = new QGraphicsRectItem(-8, -6, 16, 12, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(220, 220, 255)));
    // 基站塔符号
    QGraphicsLineItem *tower = new QGraphicsLineItem(0, -4, 0, 4, group);
    tower->setPen(standardPen());
    QGraphicsLineItem *tower2 = new QGraphicsLineItem(-3, -2, 3, -2, group);
    tower2->setPen(standardPen());
    QGraphicsLineItem *tower3 = new QGraphicsLineItem(-2, 0, 2, 0, group);
    tower3->setPen(standardPen());
    // 端口
    drawPort(group, QPointF(8, 0), "OUT");
    addModelLabel(group, "信源", QPointF(-6, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawOmniCeilingAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 全向吸顶天线：三角形+圆点
    QPolygonF triangle;
    triangle << QPointF(0, -6) << QPointF(-5, 4) << QPointF(5, 4);
    QGraphicsPolygonItem *tri = new QGraphicsPolygonItem(triangle, group);
    tri->setPen(thickPen());
    tri->setBrush(QBrush(QColor(255, 255, 200)));
    // 中心点
    QGraphicsEllipseItem *center = new QGraphicsEllipseItem(-1, -1, 2, 2, group);
    center->setBrush(QBrush(QColor(0, 0, 0)));
    center->setPen(Qt::NoPen);
    // 辐射线（全向）
    for (int i = 0; i < 8; i++) {
        qreal angle = i * M_PI / 4;
        QPointF start(4 * qCos(angle), 4 * qSin(angle));
        QPointF end(6 * qCos(angle), 6 * qSin(angle));
        QGraphicsLineItem *rad = new QGraphicsLineItem(QLineF(start, end), group);
        rad->setPen(standardPen());
    }
    addModelLabel(group, "全向吸顶", QPointF(-8, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawPanelAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 板状天线：矩形+方向箭头
    QGraphicsRectItem *rect = new QGraphicsRectItem(-4, -8, 8, 16, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(200, 255, 200)));
    // 方向箭头
    drawArrow(group, QPointF(0, 0), QPointF(8, 0));
    // 辐射线（定向）
    for (int i = -2; i <= 2; i++) {
        qreal angle = i * M_PI / 8;
        QPointF start(6 * qCos(angle), 6 * qSin(angle));
        QPointF end(9 * qCos(angle), 9 * qSin(angle));
        QGraphicsLineItem *rad = new QGraphicsLineItem(QLineF(start, end), group);
        rad->setPen(standardPen());
    }
    addModelLabel(group, "板状天线", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawWallAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 壁挂天线：小矩形+墙面线
    QGraphicsLineItem *wall = new QGraphicsLineItem(-8, -6, -8, 6, group);
    wall->setPen(QPen(QColor(0, 0, 0), 1, Qt::DashLine));
    QGraphicsRectItem *rect = new QGraphicsRectItem(-6, -4, 6, 8, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(255, 220, 200)));
    drawArrow(group, QPointF(0, 0), QPointF(6, 0));
    addModelLabel(group, "壁挂天线", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawPowerSplitter(int ways, QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 功分器：圆形+端口
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(-5, -5, 10, 10, group);
    circle->setPen(thickPen());
    circle->setBrush(QBrush(QColor(255, 255, 255)));
    // 输入端口
    drawPort(group, QPointF(-5, 0), "IN");
    // 输出端口
    if (ways == 2) {
        drawPort(group, QPointF(5, -3), "OUT1");
        drawPort(group, QPointF(5, 3), "OUT2");
    } else if (ways == 3) {
        drawPort(group, QPointF(5, -4), "OUT1");
        drawPort(group, QPointF(5, 0), "OUT2");
        drawPort(group, QPointF(5, 4), "OUT3");
    } else if (ways == 4) {
        drawPort(group, QPointF(5, -5), "OUT1");
        drawPort(group, QPointF(5, -2), "OUT2");
        drawPort(group, QPointF(5, 2), "OUT3");
        drawPort(group, QPointF(5, 5), "OUT4");
    }
    // 功分标识
    QGraphicsTextItem *label = new QGraphicsTextItem(QString("%1").arg(ways), group);
    label->setFont(QFont("SimSun", 6, QFont::Bold));
    label->setPos(-2, -4);
    addModelLabel(group, QString("%1功分").arg(ways), QPointF(-6, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawCoupler(qreal db, QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 耦合器：矩形+直通+耦合端口
    QGraphicsRectItem *rect = new QGraphicsRectItem(-6, -4, 12, 8, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(255, 255, 255)));
    // 直通线
    QGraphicsLineItem *through = new QGraphicsLineItem(-6, 0, 6, 0, group);
    through->setPen(standardPen());
    // 耦合线（向下）
    QGraphicsLineItem *couple = new QGraphicsLineItem(0, 0, 0, 6, group);
    couple->setPen(standardPen());
    // 端口
    drawPort(group, QPointF(-6, 0), "IN");
    drawPort(group, QPointF(6, 0), "OUT");
    drawPort(group, QPointF(0, 6), "CPL");
    // dB值
    QGraphicsTextItem *label = new QGraphicsTextItem(QString("%1dB").arg(db), group);
    label->setFont(QFont("SimSun", 5));
    label->setPos(-5, -10);
    addModelLabel(group, QString("耦合器%1dB").arg(db), QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawCombiner(int ways, QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 合路器：菱形+多输入
    QPolygonF diamond;
    diamond << QPointF(0, -6) << QPointF(6, 0) << QPointF(0, 6) << QPointF(-6, 0);
    QGraphicsPolygonItem *dia = new QGraphicsPolygonItem(diamond, group);
    dia->setPen(thickPen());
    dia->setBrush(QBrush(QColor(220, 255, 255)));
    // 输入端口
    for (int i = 0; i < ways; i++) {
        qreal y = -4 + i * (8.0 / (ways - 1 > 0 ? ways - 1 : 1));
        drawPort(group, QPointF(-6, y), QString("IN%1").arg(i + 1));
    }
    drawPort(group, QPointF(6, 0), "OUT");
    addModelLabel(group, QString("%1合路").arg(ways), QPointF(-6, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawLoad(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 负载：矩形+斜线
    QGraphicsRectItem *rect = new QGraphicsRectItem(-4, -4, 8, 8, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(200, 200, 200)));
    // 斜线（终端标识）
    QGraphicsLineItem *slash = new QGraphicsLineItem(-2, -2, 2, 2, group);
    slash->setPen(standardPen());
    drawPort(group, QPointF(-4, 0), "IN");
    addModelLabel(group, "负载", QPointF(-4, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawAttenuator(qreal db, QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 衰减器：矩形+锯齿
    QGraphicsRectItem *rect = new QGraphicsRectItem(-6, -3, 12, 6, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(255, 255, 255)));
    // 锯齿线
    QPainterPath path;
    path.moveTo(-5, 0);
    for (int i = 0; i < 5; i++) {
        path.lineTo(-4 + i * 2.5, (i % 2 == 0 ? -2 : 2));
    }
    path.lineTo(5, 0);
    QGraphicsPathItem *saw = new QGraphicsPathItem(path, group);
    saw->setPen(standardPen());
    drawPort(group, QPointF(-6, 0), "IN");
    drawPort(group, QPointF(6, 0), "OUT");
    QGraphicsTextItem *label = new QGraphicsTextItem(QString("%1dB").arg(db), group);
    label->setFont(QFont("SimSun", 5));
    label->setPos(-4, -8);
    addModelLabel(group, "衰减器", QPointF(-6, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawFiberAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 光纤天线：圆形+光纤标识
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(-6, -6, 12, 12, group);
    circle->setPen(thickPen());
    circle->setBrush(QBrush(QColor(255, 230, 255)));
    // 光纤符号（波浪线）
    QPainterPath path;
    path.moveTo(-4, 0);
    for (int i = 0; i < 4; i++) {
        path.quadTo(-3 + i * 2, (i % 2 == 0 ? -3 : 3), -2 + i * 2, 0);
    }
    QGraphicsPathItem *fiber = new QGraphicsPathItem(path, group);
    fiber->setPen(QPen(QColor(150, 0, 150), 0.5));
    addModelLabel(group, "光纤天线", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawLeakyCable(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 漏缆：双线+槽孔
    QGraphicsLineItem *line1 = new QGraphicsLineItem(-10, -2, 10, -2, group);
    line1->setPen(thickPen());
    QGraphicsLineItem *line2 = new QGraphicsLineItem(-10, 2, 10, 2, group);
    line2->setPen(thickPen());
    // 槽孔
    for (int i = -8; i <= 8; i += 4) {
        QGraphicsLineItem *slot = new QGraphicsLineItem(i, -2, i, 2, group);
        slot->setPen(standardPen());
    }
    addModelLabel(group, "漏缆", QPointF(-4, 6));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawSpotlightAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 射灯天线：圆形+强方向辐射
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(-5, -5, 10, 10, group);
    circle->setPen(thickPen());
    circle->setBrush(QBrush(QColor(255, 255, 200)));
    // 强方向辐射（扇形）
    QPainterPath path;
    path.moveTo(5, 0);
    path.arcTo(5, -8, 16, 16, -30, 60);
    path.closeSubpath();
    QGraphicsPathItem *beam = new QGraphicsPathItem(path, group);
    beam->setBrush(QBrush(QColor(255, 255, 0, 100)));
    beam->setPen(Qt::NoPen);
    addModelLabel(group, "射灯天线", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawExternalAntenna(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 外引天线：八木天线符号
    QGraphicsLineItem *boom = new QGraphicsLineItem(-8, 0, 8, 0, group);
    boom->setPen(thickPen());
    // 引向器
    for (int i = 0; i < 3; i++) {
        QGraphicsLineItem *dir = new QGraphicsLineItem(2 + i * 2, -3, 2 + i * 2, 3, group);
        dir->setPen(standardPen());
    }
    // 反射器
    QGraphicsLineItem *ref = new QGraphicsLineItem(-6, -5, -6, 5, group);
    ref->setPen(thickPen());
    addModelLabel(group, "外引天线", QPointF(-8, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawPRRU(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 5G pRRU：矩形+5G标识
    QGraphicsRectItem *rect = new QGraphicsRectItem(-7, -5, 14, 10, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(200, 230, 255)));
    QGraphicsTextItem *label = new QGraphicsTextItem("pRRU", group);
    label->setFont(QFont("SimSun", 6, QFont::Bold));
    label->setPos(-5, -4);
    drawPort(group, QPointF(-7, -2), "光纤");
    drawPort(group, QPointF(7, 0), "天线");
    addModelLabel(group, "5G pRRU", QPointF(-8, 8));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawRHUB(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 5G RHUB：矩形+多端口
    QGraphicsRectItem *rect = new QGraphicsRectItem(-8, -6, 16, 12, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(230, 200, 255)));
    QGraphicsTextItem *label = new QGraphicsTextItem("RHUB", group);
    label->setFont(QFont("SimSun", 6, QFont::Bold));
    label->setPos(-5, -5);
    // 多光纤端口
    for (int i = 0; i < 4; i++) {
        drawPort(group, QPointF(-8, -4 + i * 2.5), QString("P%1").arg(i + 1));
    }
    addModelLabel(group, "5G RHUB", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawBBU(QGraphicsItem *parent) {
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    // 5G BBU：大矩形+基带标识
    QGraphicsRectItem *rect = new QGraphicsRectItem(-10, -7, 20, 14, group);
    rect->setPen(thickPen());
    rect->setBrush(QBrush(QColor(255, 220, 220)));
    QGraphicsTextItem *label = new QGraphicsTextItem("BBU", group);
    label->setFont(QFont("SimSun", 7, QFont::Bold));
    label->setPos(-5, -5);
    drawPort(group, QPointF(10, 0), "光纤");
    addModelLabel(group, "5G BBU", QPointF(-8, 10));
    return group;
}

QGraphicsItemGroup* ProfessionalSymbolPainter::drawByModel(const QString &model, QGraphicsItem *parent) {
    QString m = model.toLower();
    if (m.contains("信源") || m.contains("基站") || m.contains("omb") || m.contains("bts")) {
        return drawSource(parent);
    } else if (m.contains("全向") || m.contains("吸顶") || m.contains("omni")) {
        return drawOmniCeilingAntenna(parent);
    } else if (m.contains("板状") || m.contains("定向") || m.contains("panel")) {
        return drawPanelAntenna(parent);
    } else if (m.contains("壁挂") || m.contains("wall")) {
        return drawWallAntenna(parent);
    } else if (m.contains("二功分") || m.contains("2功分") || m.contains("pow-2")) {
        return drawPowerSplitter(2, parent);
    } else if (m.contains("三功分") || m.contains("3功分") || m.contains("pow-3")) {
        return drawPowerSplitter(3, parent);
    } else if (m.contains("四功分") || m.contains("4功分") || m.contains("pow-4")) {
        return drawPowerSplitter(4, parent);
    } else if (m.contains("耦合器") || m.contains("cou")) {
        qreal db = 10;
        if (m.contains("5db")) db = 5;
        else if (m.contains("6db")) db = 6;
        else if (m.contains("7db")) db = 7;
        else if (m.contains("10db")) db = 10;
        else if (m.contains("12db")) db = 12;
        else if (m.contains("15db")) db = 15;
        else if (m.contains("20db")) db = 20;
        else if (m.contains("30db")) db = 30;
        else if (m.contains("40db")) db = 40;
        return drawCoupler(db, parent);
    } else if (m.contains("合路") || m.contains("com")) {
        int ways = 2;
        if (m.contains("三合路") || m.contains("3合路")) ways = 3;
        else if (m.contains("四合路") || m.contains("4合路")) ways = 4;
        return drawCombiner(ways, parent);
    } else if (m.contains("负载") || m.contains("load")) {
        return drawLoad(parent);
    } else if (m.contains("衰减") || m.contains("att")) {
        return drawAttenuator(10, parent);
    } else if (m.contains("光纤") || m.contains("fiber")) {
        return drawFiberAntenna(parent);
    } else if (m.contains("漏缆") || m.contains("leaky")) {
        return drawLeakyCable(parent);
    } else if (m.contains("射灯") || m.contains("spotlight")) {
        return drawSpotlightAntenna(parent);
    } else if (m.contains("外引") || m.contains("external")) {
        return drawExternalAntenna(parent);
    } else if (m.contains("prru")) {
        return drawPRRU(parent);
    } else if (m.contains("rhub")) {
        return drawRHUB(parent);
    } else if (m.contains("bbu")) {
        return drawBBU(parent);
    }
    // 默认：矩形
    QGraphicsItemGroup *group = new QGraphicsItemGroup(parent);
    QGraphicsRectItem *rect = new QGraphicsRectItem(-5, -5, 10, 10, group);
    rect->setPen(standardPen());
    addModelLabel(group, model, QPointF(-8, 8));
    return group;
}

} // namespace Zhifen
