#include "deviceitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QPen>
#include <QBrush>

namespace Zhifen {

DeviceItem::DeviceItem(DeviceType type, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_type(type)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

DeviceItem::~DeviceItem()
{
}

QString DeviceItem::deviceName() const
{
    return deviceTypeName(m_type);
}

QString DeviceItem::deviceTypeName(DeviceType type)
{
    switch (type) {
    case OmniAntenna: return "全向天线";
    case DirectionalAntenna: return "定向天线";
    case SpotlightAntenna: return "射灯天线";
    case ExternalAntenna: return "外引天线";
    case WallMountAntenna: return "壁挂天线";
    case Coupler: return "耦合器";
    case Splitter: return "功分器";
    case Combiner: return "合路器";
    case Hybrid: return "电桥";
    case Attenuator: return "衰减器";
    case Load: return "负载";
    case Isolator: return "隔离器";
    case Circulator: return "环形器";
    case MacroBS: return "宏基站";
    case MicroBS: return "微基站";
    case FiberRepeater: return "光纤直放站";
    case BBU: return "BBU";
    case RRU: return "RRU";
    case PicoStation: return "皮基站";
    case Ground: return "接地";
    case LightningProtector: return "防雷器";
    case Switch: return "开关";
    case Connector: return "接头";
    default: return "未知器件";
    }
}

QRectF DeviceItem::boundingRect() const
{
    qreal s = m_scale;
    switch (m_type) {
    case OmniAntenna:
    case DirectionalAntenna:
    case SpotlightAntenna:
    case ExternalAntenna:
    case WallMountAntenna:
        return DeviceSymbols::antennaRect(s);
    case Coupler:
        return DeviceSymbols::couplerRect(s);
    case Splitter:
        return DeviceSymbols::splitterRect(s);
    case Combiner:
    case Hybrid:
        return DeviceSymbols::combinerRect(s);
    case MacroBS:
    case MicroBS:
    case FiberRepeater:
    case BBU:
    case RRU:
    case PicoStation:
        return DeviceSymbols::sourceRect(s);
    default:
        return QRectF(-3 * s, -3 * s, 6 * s, 6 * s);
    }
}

void DeviceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();

    // 选中状态：绘制选中框
    if (isSelected()) {
        painter->setPen(QPen(QColor(0, 120, 255), 0.2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(-1, -1, 1, 1));
    }

    // 根据类型绘制标准器件图元
    switch (m_type) {
    case OmniAntenna:
        DeviceSymbols::drawOmniAntenna(painter, m_scale);
        break;
    case DirectionalAntenna:
        DeviceSymbols::drawDirectionalAntenna(painter, m_scale);
        break;
    case SpotlightAntenna:
        DeviceSymbols::drawSpotlightAntenna(painter, m_scale);
        break;
    case ExternalAntenna:
        DeviceSymbols::drawExternalAntenna(painter, m_scale);
        break;
    case WallMountAntenna:
        DeviceSymbols::drawWallMountAntenna(painter, m_scale);
        break;
    case Coupler:
        DeviceSymbols::drawCoupler(painter, m_scale, m_couplerDb);
        break;
    case Splitter:
        DeviceSymbols::drawSplitter(painter, m_scale, m_splitterWays);
        break;
    case Combiner:
        DeviceSymbols::drawCombiner(painter, m_scale);
        break;
    case Hybrid:
        DeviceSymbols::drawHybrid(painter, m_scale);
        break;
    case Attenuator:
        DeviceSymbols::drawAttenuator(painter, m_scale, m_attenuatorDb);
        break;
    case Load:
        DeviceSymbols::drawLoad(painter, m_scale);
        break;
    case Isolator:
        DeviceSymbols::drawIsolator(painter, m_scale);
        break;
    case Circulator:
        DeviceSymbols::drawCirculator(painter, m_scale);
        break;
    case MacroBS:
        DeviceSymbols::drawMacroBS(painter, m_scale);
        break;
    case MicroBS:
        DeviceSymbols::drawMicroBS(painter, m_scale);
        break;
    case FiberRepeater:
        DeviceSymbols::drawFiberRepeater(painter, m_scale);
        break;
    case BBU:
        DeviceSymbols::drawBBU(painter, m_scale);
        break;
    case RRU:
        DeviceSymbols::drawRRU(painter, m_scale);
        break;
    case PicoStation:
        DeviceSymbols::drawPicoStation(painter, m_scale);
        break;
    case Ground:
        DeviceSymbols::drawGround(painter, m_scale);
        break;
    case LightningProtector:
        DeviceSymbols::drawLightningProtector(painter, m_scale);
        break;
    case Switch:
        DeviceSymbols::drawSwitch(painter, m_scale);
        break;
    case Connector:
        DeviceSymbols::drawConnector(painter, m_scale);
        break;
    }

    painter->restore();
}

QList<QPointF> DeviceItem::connectionPoints() const
{
    QList<QPointF> points;
    qreal s = m_scale;

    switch (m_type) {
    case OmniAntenna:
        points << QPointF(0, 2.5 * s);
        break;
    case DirectionalAntenna:
        points << QPointF(-2 * s, 0);
        break;
    case Coupler:
        points << DeviceSymbols::couplerInput(s)
               << DeviceSymbols::couplerOutput(s)
               << DeviceSymbols::couplerCoupledPort(s);
        break;
    case Splitter:
        points << QPointF(-4 * s, 0);
        if (m_splitterWays == 2) {
            points << QPointF(2 * s, -2 * s) << QPointF(2 * s, 2 * s);
        } else if (m_splitterWays == 3) {
            points << QPointF(2 * s, -2.5 * s) << QPointF(2 * s, 0) << QPointF(2 * s, 2.5 * s);
        } else {
            points << QPointF(2 * s, -3 * s) << QPointF(2 * s, -1 * s)
                   << QPointF(2 * s, 1 * s) << QPointF(2 * s, 3 * s);
        }
        break;
    case Combiner:
        points << QPointF(-5 * s, -1.5 * s) << QPointF(-5 * s, 1.5 * s) << QPointF(5 * s, 0);
        break;
    case Hybrid:
        points << QPointF(-3 * s, -1.5 * s) << QPointF(-3 * s, 1.5 * s)
               << QPointF(3 * s, -1.5 * s) << QPointF(3 * s, 1.5 * s);
        break;
    default:
        points << QPointF(0, 0);
        break;
    }

    return points;
}

QVariant DeviceItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItem::itemChange(change, value);
}

} // namespace Zhifen
