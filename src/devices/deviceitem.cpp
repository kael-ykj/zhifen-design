#include "deviceitem.h"
#include "core/professional_symbols.h"
#include <QPainter>
#include <cmath>

DeviceItem::DeviceItem(DeviceType type, QGraphicsItem *parent)
    : CadItem(parent), m_deviceType(type)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    initDeviceProperties();
    setupConnectPoints();
    // 创建专业符号
    m_symbolGroup = Zhifen::ProfessionalSymbolPainter::drawByModel(deviceTypeName(), this);
    if (m_symbolGroup) m_symbolGroup->setFlag(QGraphicsItem::ItemIsSelectable, false);
}

QString DeviceItem::deviceTypeName() const
{
    switch (m_deviceType) {
    case DevAntennaOmni: return "全向吸顶天线";
    case DevAntennaDirectional: return "定向壁挂天线";
    case DevAntennaLPDA: return "对数周期天线";
    case DevSplitter2: return "二功分器";
    case DevSplitter3: return "三功分器";
    case DevSplitter4: return "四功分器";
    case DevCoupler5: return "5dB耦合器";
    case DevCoupler6: return "6dB耦合器";
    case DevCoupler7: return "7dB耦合器";
    case DevCoupler10: return "10dB耦合器";
    case DevCoupler12: return "12dB耦合器";
    case DevCoupler15: return "15dB耦合器";
    case DevCoupler20: return "20dB耦合器";
    case DevCombiner: return "合路器";
    case DevFeederHalf: return "1/2馈线";
    case DevFeeder78: return "7/8馈线";
    case DevFiber: return "光纤";
    case DevSourceRRU: return "RRU";
    case DevSourceBBU: return "BBU";
    case DevSourceMicro: return "微基站";
    case DevSourceRepeater: return "直放站";
    case DevDryAmp: return "干放";
    case DevHybrid: return "3dB电桥";
    case DevLoad: return "终端负载";
    case DevAttenuator: return "衰减器";
    case DevLightning: return "避雷器";
    case DevCoupler25: return "25dB耦合器";
    case DevCoupler30: return "30dB耦合器";
    case DevCoupler40: return "40dB耦合器";
    case DevFeeder158: return "1-5/8馈线";
    case DevFeeder5D: return "5D-FB馈线";
    case DevFeeder8D: return "8D-FB馈线";
    case DevNetworkCable: return "网线";
    case DevLeakyCable158: return "1-5/8漏缆";
    case DevLeakyCable138: return "13/8漏缆";
    case DevAntennaSpotlight: return "射灯天线";
    case DevAntennaExternal: return "外引天线";
    case DevAntennaPanel: return "板状天线";
    case DevAntennaYagi: return "八木天线";
    case DevAntennaGrid: return "栅格天线";
    case DevpRRU: return "pRRU皮基站";
    case DevRHUB: return "RHUB射频集线器";
    case DevPOESwitch: return "POE交换机";
    default: return "未知器件";
    }
}

void DeviceItem::initDeviceProperties()
{
    switch (m_deviceType) {
    case DevAntennaOmni:
        m_model = "ANT-OMNI-800/2700";
        m_insertionLoss = 0;
        m_properties["频段"] = "800-2700MHz";
        m_properties["增益"] = "2dBi";
        m_properties["极化方式"] = "垂直";
        setLayer("天线");
        break;
    case DevAntennaDirectional:
        m_model = "ANT-DIR-800/2700";
        m_insertionLoss = 0;
        m_properties["频段"] = "800-2700MHz";
        m_properties["增益"] = "7dBi";
        m_properties["半功率角"] = "65°";
        setLayer("天线");
        break;
    case DevAntennaLPDA:
        m_model = "ANT-LPDA-800/2500";
        m_insertionLoss = 0;
        m_properties["频段"] = "800-2500MHz";
        m_properties["增益"] = "9dBi";
        setLayer("天线");
        break;
    case DevSplitter2:
        m_model = "PS-2-Way";
        m_insertionLoss = 3.5;
        m_properties["频段"] = "800-2700MHz";
        m_properties["分配损耗"] = "3.0dB";
        m_properties["插入损耗"] = "0.5dB";
        setLayer("器件");
        break;
    case DevSplitter3:
        m_model = "PS-3-Way";
        m_insertionLoss = 5.5;
        m_properties["频段"] = "800-2700MHz";
        m_properties["分配损耗"] = "4.8dB";
        m_properties["插入损耗"] = "0.7dB";
        setLayer("器件");
        break;
    case DevSplitter4:
        m_model = "PS-4-Way";
        m_insertionLoss = 6.8;
        m_properties["频段"] = "800-2700MHz";
        m_properties["分配损耗"] = "6.0dB";
        m_properties["插入损耗"] = "0.8dB";
        setLayer("器件");
        break;
    case DevCoupler5:
        m_model = "CPL-5dB";
        m_insertionLoss = 1.8;
        m_properties["耦合度"] = "5dB";
        m_properties["插入损耗"] = "1.8dB";
        setLayer("器件");
        break;
    case DevCoupler6:
        m_model = "CPL-6dB";
        m_insertionLoss = 1.5;
        m_properties["耦合度"] = "5dB";
        m_properties["插入损耗"] = "1.8dB";
        setLayer("器件");
        break;
    case DevCoupler7:
        m_model = "CPL-7dB";
        m_insertionLoss = 1.4;
        m_properties["耦合度"] = "7dB";
        m_properties["插入损耗"] = "1.4dB";
        setLayer("器件");
        break;
    case DevCoupler10:
        m_model = "CPL-10dB";
        m_insertionLoss = 0.8;
        m_properties["耦合度"] = "10dB";
        m_properties["插入损耗"] = "0.8dB";
        setLayer("器件");
        break;
    case DevCoupler12:
        m_model = "CPL-12dB";
        m_insertionLoss = 0.6;
        m_properties["耦合度"] = "10dB";
        m_properties["插入损耗"] = "0.8dB";
        setLayer("器件");
        break;
    case DevCoupler15:
        m_model = "CPL-15dB";
        m_insertionLoss = 0.4;
        m_properties["耦合度"] = "15dB";
        m_properties["插入损耗"] = "0.4dB";
        setLayer("器件");
        break;
    case DevCoupler20:
        m_model = "CPL-20dB";
        m_insertionLoss = 0.2;
        m_properties["耦合度"] = "20dB";
        m_properties["插入损耗"] = "0.2dB";
        setLayer("器件");
        break;
    case DevCombiner:
        m_model = "CB-3Way";
        m_insertionLoss = 0.5;
        m_properties["频段"] = "800-2700MHz";
        m_properties["插入损耗"] = "0.5dB";
        m_properties["隔离度"] = "25dB";
        setLayer("器件");
        break;
    case DevFeederHalf:
        m_model = "1/2-Feeder";
        m_insertionLoss = 0.0; // 按长度计算
        m_properties["类型"] = "1/2馈线";
        m_properties["损耗(900MHz)"] = "6.1dB/100m";
        m_properties["损耗(2000MHz)"] = "9.5dB/100m";
        setLayer("馈线");
        break;
    case DevFeeder78:
        m_model = "7/8-Feeder";
        m_insertionLoss = 0.0;
        m_properties["类型"] = "7/8馈线";
        m_properties["损耗(900MHz)"] = "3.9dB/100m";
        m_properties["损耗(2000MHz)"] = "6.1dB/100m";
        setLayer("馈线");
        break;
    case DevFiber:
        m_model = "Fiber-SM";
        m_insertionLoss = 0.0;
        m_properties["类型"] = "单模光纤";
        m_properties["损耗"] = "0.4dB/100m";
        setLayer("光纤");
        break;
    case DevSourceRRU:
        m_model = "RRU-4T4R";
        m_power = 43.0; // 43dBm = 20W
        m_properties["类型"] = "射频拉远单元";
        m_properties["发射功率"] = "2x40W";
        m_properties["频段"] = "可配置";
        setLayer("器件");
        break;
    case DevSourceBBU:
        m_model = "BBU-5G";
        m_properties["类型"] = "基带处理单元";
        m_properties["制式"] = "5G NR";
        setLayer("器件");
        break;
    case DevSourceMicro:
        m_model = "Micro-BTS";
        m_power = 37.0; // 37dBm = 5W
        m_properties["类型"] = "微基站";
        m_properties["发射功率"] = "5W";
        setLayer("器件");
        break;
    case DevSourceRepeater:
        m_model = "Repeater-2W";
        m_power = 33.0; // 33dBm = 2W
        m_properties["类型"] = "直放站";
        m_properties["增益"] = "80dB";
        setLayer("器件");
        break;
    default:
        break;
    }
}

void DeviceItem::setupConnectPoints()
{
    m_connectPoints.clear();
    qreal s = m_size;
    switch (m_deviceType) {
    case DevAntennaOmni:
    case DevAntennaDirectional:
    case DevAntennaLPDA:
        m_connectPoints.append({QPointF(0, -s), "输入", false});
        break;
    case DevSplitter2:
        m_connectPoints.append({QPointF(0, -s), "输入", false});
        m_connectPoints.append({QPointF(-s*0.7, s), "输出1", false});
        m_connectPoints.append({QPointF(s*0.7, s), "输出2", false});
        break;
    case DevSplitter3:
        m_connectPoints.append({QPointF(0, -s), "输入", false});
        m_connectPoints.append({QPointF(-s, s), "输出1", false});
        m_connectPoints.append({QPointF(0, s), "输出2", false});
        m_connectPoints.append({QPointF(s, s), "输出3", false});
        break;
    case DevSplitter4:
        m_connectPoints.append({QPointF(0, -s), "输入", false});
        m_connectPoints.append({QPointF(-s*1.2, s), "输出1", false});
        m_connectPoints.append({QPointF(-s*0.4, s), "输出2", false});
        m_connectPoints.append({QPointF(s*0.4, s), "输出3", false});
        m_connectPoints.append({QPointF(s*1.2, s), "输出4", false});
        break;
    case DevCoupler5:
    case DevCoupler6:
    case DevCoupler7:
    case DevCoupler10:
    case DevCoupler12:
    case DevCoupler15:
    case DevCoupler20:
    case DevCoupler25:
    case DevCoupler30:
    case DevCoupler40:
        m_connectPoints.append({QPointF(-s, 0), "输入", false});
        m_connectPoints.append({QPointF(s, 0), "输出", false});
        m_connectPoints.append({QPointF(0, s), "耦合端", false});
        break;
    case DevCombiner:
        m_connectPoints.append({QPointF(-s, -s*0.5), "输入1", false});
        m_connectPoints.append({QPointF(-s, s*0.5), "输入2", false});
        m_connectPoints.append({QPointF(s, 0), "输出", false});
        break;
    case DevSourceRRU:
    case DevSourceMicro:
    case DevSourceRepeater:
    case DevDryAmp:
    case DevpRRU:
        m_connectPoints.append({QPointF(s, 0), "输出", false});
        break;
    case DevHybrid:
        m_connectPoints.append({QPointF(-s, -s*0.5), "输入1", false});
        m_connectPoints.append({QPointF(-s, s*0.5), "输入2", false});
        m_connectPoints.append({QPointF(s, -s*0.5), "输出1", false});
        m_connectPoints.append({QPointF(s, s*0.5), "输出2", false});
        break;
    case DevLoad:
    case DevAttenuator:
    case DevLightning:
        m_connectPoints.append({QPointF(-s, 0), "输入", false});
        m_connectPoints.append({QPointF(s, 0), "输出", false});
        break;
    case DevRHUB:
        m_connectPoints.append({QPointF(0, -s), "上联", false});
        for (int i = 0; i < 4; i++)
            m_connectPoints.append({QPointF(-s*0.6 + i*s*0.4, s), QString("下联%1").arg(i+1), false});
        break;
    case DevPOESwitch:
        m_connectPoints.append({QPointF(0, -s), "上联", false});
        for (int i = 0; i < 8; i++)
            m_connectPoints.append({QPointF(-s*0.7 + i*s*0.2, s), QString("POE%1").arg(i+1), false});
        break;
    default:
        break;
    }
}

qreal DeviceItem::distanceToPoint(const QPointF &p) const
{
    return QLineF(p, pos()).length();
}

QPen DeviceItem::devicePen() const
{
    QPen pen = effectivePen();
    pen.setWidthF(1.5);
    return pen;
}

QRectF DeviceItem::boundingRect() const
{
    qreal s = m_size * 1.5;
    return QRectF(-s, -s, s*2, s*2);
}

QPainterPath DeviceItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void DeviceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // 专业符号由m_symbolGroup子项绘制
    // 只绘制选中状态和型号标签
    if (isSelected()) {
        painter->setPen(QPen(QColor(0, 120, 255), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
    // 绘制型号标签
    painter->setPen(QPen(QColor(0, 0, 0)));
    painter->setFont(QFont("SimSun", m_size * 0.35));
    painter->drawText(QRectF(-m_size, m_size * 0.9, m_size * 2, m_size * 0.4),
                      Qt::AlignCenter, m_model.isEmpty() ? deviceTypeName() : m_model);
}
