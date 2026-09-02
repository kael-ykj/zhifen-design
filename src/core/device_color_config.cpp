#include "device_color_config.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>

namespace Zhifen {

DeviceColorConfig& DeviceColorConfig::instance()
{
    static DeviceColorConfig inst;
    return inst;
}

DeviceColorConfig::DeviceColorConfig()
{
    // 默认迪弗风格
    applyDifuTheme();
}

void DeviceColorConfig::applyDifuTheme()
{
    // 迪弗软件风格：蓝馈线/红器件/红天线/紫标注/绿功率/白墙体
    m_feederColor = QColor(0, 0, 255);      // 蓝色馈线
    m_deviceColor = QColor(255, 0, 0);      // 红色器件
    m_antennaColor = QColor(255, 0, 0);     // 红色天线
    m_labelColor = QColor(255, 0, 255);     // 紫色标注
    m_powerColor = QColor(0, 255, 0);       // 绿色功率
    m_wallColor = QColor(255, 255, 255);    // 白色墙体
    m_textColor = QColor(255, 255, 255);    // 白色文字
}

void DeviceColorConfig::applyTianyueTheme()
{
    // 天越软件风格
    m_feederColor = QColor(0, 255, 0);      // 绿色馈线
    m_deviceColor = QColor(255, 255, 0);    // 黄色器件
    m_antennaColor = QColor(255, 0, 0);     // 红色天线
    m_labelColor = QColor(0, 255, 255);     // 青色标注
    m_powerColor = QColor(255, 255, 0);     // 黄色功率
    m_wallColor = QColor(255, 255, 255);    // 白色墙体
    m_textColor = QColor(255, 255, 255);    // 白色文字
}

void DeviceColorConfig::applyDefaultTheme()
{
    // 默认风格
    m_feederColor = QColor(0, 200, 0);
    m_deviceColor = QColor(255, 200, 0);
    m_antennaColor = QColor(255, 80, 80);
    m_labelColor = QColor(200, 200, 200);
    m_powerColor = QColor(0, 255, 0);
    m_wallColor = QColor(255, 255, 255);
    m_textColor = QColor(255, 255, 255);
}

bool DeviceColorConfig::save(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("colors/feeder", m_feederColor.name());
    settings.setValue("colors/device", m_deviceColor.name());
    settings.setValue("colors/antenna", m_antennaColor.name());
    settings.setValue("colors/label", m_labelColor.name());
    settings.setValue("colors/power", m_powerColor.name());
    settings.setValue("colors/wall", m_wallColor.name());
    settings.setValue("colors/text", m_textColor.name());
    return true;
}

bool DeviceColorConfig::load(const QString &path)
{
    QSettings settings(path, QSettings::IniFormat);
    if (settings.contains("colors/feeder")) {
        m_feederColor = QColor(settings.value("colors/feeder").toString());
        m_deviceColor = QColor(settings.value("colors/device").toString());
        m_antennaColor = QColor(settings.value("colors/antenna").toString());
        m_labelColor = QColor(settings.value("colors/label").toString());
        m_powerColor = QColor(settings.value("colors/power").toString());
        m_wallColor = QColor(settings.value("colors/wall").toString());
        m_textColor = QColor(settings.value("colors/text").toString());
        return true;
    }
    return false;
}

} // namespace Zhifen
