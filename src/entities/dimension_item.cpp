#include "dimension_item.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QtMath>
#include <QFontMetricsF>

namespace Zhifen {

// ==================== DimensionItem基类 ====================
DimensionItem::DimensionItem(DimensionType type, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_type(type)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

DimensionItem::~DimensionItem() {}

QString DimensionItem::autoText() const {
    qreal val = measuredValue();
    QString num = QString::number(val, 'f', m_style.precision);
    return m_style.prefix + num + m_style.suffix;
}

QRectF DimensionItem::boundingRect() const {
    return m_boundingRect;
}

void DimensionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // 子类实现
}

void DimensionItem::drawArrow(QPainter *painter, const QPointF &pos, qreal angle) {
    qreal size = m_style.arrowSize;
    painter->save();
    painter->translate(pos);
    painter->rotate(angle);
    QPolygonF arrow;
    arrow << QPointF(0, 0)
          << QPointF(-size, size * 0.4)
          << QPointF(-size, -size * 0.4);
    painter->setBrush(QBrush(m_style.color));
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(arrow);
    painter->restore();
}

void DimensionItem::drawText(QPainter *painter, const QPointF &pos, const QString &text) {
    painter->save();
    QFont font = painter->font();
    font.setPointSizeF(m_style.textHeight);
    painter->setFont(font);
    painter->setPen(m_style.color);
    painter->drawText(pos, text);
    painter->restore();
}

void DimensionItem::updateBoundingRect() {
    m_boundingRect = QRectF(-50, -50, 100, 100); // 子类重写
}

// ==================== 线性标注 ====================
LinearDimension::LinearDimension(QGraphicsItem *parent)
    : DimensionItem(Dim_Linear, parent) {}

void LinearDimension::setPoints(const QPointF &p1, const QPointF &p2, const QPointF &dimPos) {
    m_p1 = p1;
    m_p2 = p2;
    m_dimPos = dimPos;
    if (m_text.isEmpty()) m_text = autoText();
    updateBoundingRect();
    update();
}

qreal LinearDimension::measuredValue() const {
    if (m_horizontal) return qAbs(m_p2.x() - m_p1.x());
    return qAbs(m_p2.y() - m_p1.y());
}

void LinearDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(m_style.color, 0.5));

    qreal dimVal = m_horizontal ? m_dimPos.y() : m_dimPos.x();

    // 尺寸界线
    QPointF ext1 = m_horizontal ? QPointF(m_p1.x(), dimVal) : QPointF(dimVal, m_p1.y());
    QPointF ext2 = m_horizontal ? QPointF(m_p2.x(), dimVal) : QPointF(dimVal, m_p2.y());
    QPointF start1 = m_horizontal ? QPointF(m_p1.x(), m_p1.y()) : QPointF(m_p1.x(), m_p1.y());
    QPointF start2 = m_horizontal ? QPointF(m_p2.x(), m_p2.y()) : QPointF(m_p2.x(), m_p2.y());

    painter->drawLine(start1, ext1);
    painter->drawLine(start2, ext2);

    // 标注线
    painter->drawLine(ext1, ext2);

    // 箭头
    qreal angle1 = m_horizontal ? 180 : 90;
    qreal angle2 = m_horizontal ? 0 : -90;
    drawArrow(painter, ext1, angle1);
    drawArrow(painter, ext2, angle2);

    // 文字
    QPointF textPos = (ext1 + ext2) / 2;
    textPos += m_horizontal ? QPointF(0, -m_style.textHeight) : QPointF(m_style.textHeight, 0);
    drawText(painter, textPos, m_text);
}

void LinearDimension::updateBoundingRect() {
    qreal minX = qMin(m_p1.x(), m_p2.x()) - 10;
    qreal maxX = qMax(m_p1.x(), m_p2.x()) + 10;
    qreal minY = qMin(m_p1.y(), m_p2.y()) - 10;
    qreal maxY = qMax(m_p1.y(), m_p2.y()) + 10;
    if (m_horizontal) {
        minY = qMin(minY, m_dimPos.y() - 10);
        maxY = qMax(maxY, m_dimPos.y() + 10);
    } else {
        minX = qMin(minX, m_dimPos.x() - 10);
        maxX = qMax(maxX, m_dimPos.x() + 10);
    }
    m_boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

// ==================== 对齐标注 ====================
AlignedDimension::AlignedDimension(QGraphicsItem *parent)
    : DimensionItem(Dim_Aligned, parent) {}

void AlignedDimension::setPoints(const QPointF &p1, const QPointF &p2, const QPointF &dimPos) {
    m_p1 = p1;
    m_p2 = p2;
    m_dimPos = dimPos;
    if (m_text.isEmpty()) m_text = autoText();
    updateBoundingRect();
    update();
}

qreal AlignedDimension::measuredValue() const {
    return qSqrt(qPow(m_p2.x() - m_p1.x(), 2) + qPow(m_p2.y() - m_p1.y(), 2));
}

void AlignedDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(m_style.color, 0.5));

    // 计算标注线位置（平行于p1-p2，偏移到dimPos）
    QLineF baseLine(m_p1, m_p2);
    qreal angle = baseLine.angle();
    QLineF normal = baseLine.normalVector();
    QPointF offset = m_dimPos - baseLine.pointAt(0.5);
    qreal dist = QLineF(baseLine.pointAt(0.5), m_dimPos).length();
    QPointF dir = normal.p2() - normal.p1();
    qreal len = qSqrt(dir.x()*dir.x() + dir.y()*dir.y());
    dir /= len;
    QPointF ext1 = m_p1 + dir * dist;
    QPointF ext2 = m_p2 + dir * dist;

    // 尺寸界线
    painter->drawLine(m_p1, ext1);
    painter->drawLine(m_p2, ext2);

    // 标注线
    painter->drawLine(ext1, ext2);

    // 箭头
    drawArrow(painter, ext1, angle + 180);
    drawArrow(painter, ext2, angle);

    // 文字
    QPointF textPos = (ext1 + ext2) / 2;
    painter->save();
    painter->translate(textPos);
    painter->rotate(-angle);
    drawText(painter, QPointF(-10, -5), m_text);
    painter->restore();
}

void AlignedDimension::updateBoundingRect() {
    qreal minX = qMin(m_p1.x(), m_p2.x()) - 20;
    qreal maxX = qMax(m_p1.x(), m_p2.x()) + 20;
    qreal minY = qMin(m_p1.y(), m_p2.y()) - 20;
    qreal maxY = qMax(m_p1.y(), m_p2.y()) + 20;
    m_boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

// ==================== 半径标注 ====================
RadiusDimension::RadiusDimension(QGraphicsItem *parent)
    : DimensionItem(Dim_Radius, parent) {}

void RadiusDimension::setCircle(const QPointF &center, qreal radius, const QPointF &dimPos) {
    m_center = center;
    m_radius = radius;
    m_dimPos = dimPos;
    if (m_text.isEmpty()) m_text = "R" + autoText();
    updateBoundingRect();
    update();
}

void RadiusDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(m_style.color, 0.5));

    // 从圆心到标注点的线
    painter->drawLine(m_center, m_dimPos);

    // 箭头
    QLineF line(m_center, m_dimPos);
    drawArrow(painter, m_dimPos, line.angle());

    // 文字
    drawText(painter, m_dimPos + QPointF(5, -5), m_text);
}

void RadiusDimension::updateBoundingRect() {
    qreal minX = qMin(m_center.x(), m_dimPos.x()) - 20;
    qreal maxX = qMax(m_center.x(), m_dimPos.x()) + 20;
    qreal minY = qMin(m_center.y(), m_dimPos.y()) - 20;
    qreal maxY = qMax(m_center.y(), m_dimPos.y()) + 20;
    m_boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

// ==================== 直径标注 ====================
DiameterDimension::DiameterDimension(QGraphicsItem *parent)
    : DimensionItem(Dim_Diameter, parent) {}

void DiameterDimension::setCircle(const QPointF &center, qreal radius, const QPointF &dimPos) {
    m_center = center;
    m_radius = radius;
    m_dimPos = dimPos;
    if (m_text.isEmpty()) m_text = "%%c" + autoText();
    updateBoundingRect();
    update();
}

void DiameterDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(m_style.color, 0.5));

    // 通过圆心的直径线
    QLineF line(m_dimPos, m_center * 2 - m_dimPos);
    painter->drawLine(line);

    // 两端箭头
    drawArrow(painter, line.p1(), line.angle());
    drawArrow(painter, line.p2(), line.angle() + 180);

    // 文字
    drawText(painter, m_dimPos + QPointF(5, -5), m_text);
}

void DiameterDimension::updateBoundingRect() {
    QPointF p2 = m_center * 2 - m_dimPos;
    qreal minX = qMin(m_dimPos.x(), p2.x()) - 20;
    qreal maxX = qMax(m_dimPos.x(), p2.x()) + 20;
    qreal minY = qMin(m_dimPos.y(), p2.y()) - 20;
    qreal maxY = qMax(m_dimPos.y(), p2.y()) + 20;
    m_boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

// ==================== 角度标注 ====================
AngularDimension::AngularDimension(QGraphicsItem *parent)
    : DimensionItem(Dim_Angular, parent) {}

void AngularDimension::setLines(const QPointF &vertex, const QPointF &p1, const QPointF &p2) {
    m_vertex = vertex;
    m_p1 = p1;
    m_p2 = p2;
    if (m_text.isEmpty()) m_text = autoText() + "%%d";
    updateBoundingRect();
    update();
}

qreal AngularDimension::measuredValue() const {
    QLineF l1(m_vertex, m_p1);
    QLineF l2(m_vertex, m_p2);
    return l1.angleTo(l2);
}

void AngularDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setPen(QPen(m_style.color, 0.5));

    qreal radius = 30;
    QLineF l1(m_vertex, m_p1);
    QLineF l2(m_vertex, m_p2);
    qreal a1 = l1.angle();
    qreal a2 = l2.angle();

    // 角度弧
    QRectF arcRect(m_vertex.x() - radius, m_vertex.y() - radius, radius * 2, radius * 2);
    painter->drawArc(arcRect, (int)(-a1 * 16), (int)((a1 - a2) * 16));

    // 延长线
    painter->drawLine(m_vertex, m_p1);
    painter->drawLine(m_vertex, m_p2);

    // 箭头
    qreal midAngle = (a1 + a2) / 2;
    QPointF arcPoint = m_vertex + QPointF(radius * qCos(qDegreesToRadians(-midAngle)),
                                          radius * qSin(qDegreesToRadians(-midAngle)));
    drawArrow(painter, arcPoint, midAngle + 90);

    // 文字
    QPointF textPos = m_vertex + QPointF((radius + 10) * qCos(qDegreesToRadians(-midAngle)),
                                         (radius + 10) * qSin(qDegreesToRadians(-midAngle)));
    drawText(painter, textPos, m_text);
}

void AngularDimension::updateBoundingRect() {
    qreal minX = qMin(m_vertex.x(), qMin(m_p1.x(), m_p2.x())) - 40;
    qreal maxX = qMax(m_vertex.x(), qMax(m_p1.x(), m_p2.x())) + 40;
    qreal minY = qMin(m_vertex.y(), qMin(m_p1.y(), m_p2.y())) - 40;
    qreal maxY = qMax(m_vertex.y(), qMax(m_p1.y(), m_p2.y())) + 40;
    m_boundingRect = QRectF(minX, minY, maxX - minX, maxY - minY);
}

} // namespace Zhifen
