#ifndef DEVICE_COLOR_CONFIG_H
#define DEVICE_COLOR_CONFIG_H

#include <QColor>
#include <QString>
#include <QMap>

namespace Zhifen {

// 器件颜色配置管理器
// 对标迪弗软件：所有器件和馈线颜色可自定义
class DeviceColorConfig
{
public:
    static DeviceColorConfig& instance();

    // 获取颜色
    QColor feederColor() const { return m_feederColor; }
    QColor deviceColor() const { return m_deviceColor; }
    QColor antennaColor() const { return m_antennaColor; }
    QColor labelColor() const { return m_labelColor; }
    QColor powerColor() const { return m_powerColor; }
    QColor wallColor() const { return m_wallColor; }
    QColor textColor() const { return m_textColor; }

    // 设置颜色
    void setFeederColor(const QColor &c) { m_feederColor = c; }
    void setDeviceColor(const QColor &c) { m_deviceColor = c; }
    void setAntennaColor(const QColor &c) { m_antennaColor = c; }
    void setLabelColor(const QColor &c) { m_labelColor = c; }
    void setPowerColor(const QColor &c) { m_powerColor = c; }
    void setWallColor(const QColor &c) { m_wallColor = c; }
    void setTextColor(const QColor &c) { m_textColor = c; }

    // 预设主题
    void applyDifuTheme();   // 迪弗风格（蓝馈线/红器件/绿功率/紫标注）
    void applyTianyueTheme(); // 天越风格
    void applyDefaultTheme(); // 默认风格

    // 保存/加载
    bool save(const QString &path);
    bool load(const QString &path);

private:
    DeviceColorConfig();

    QColor m_feederColor;    // 馈线颜色
    QColor m_deviceColor;    // 器件颜色（耦合器/功分器等）
    QColor m_antennaColor;   // 天线颜色
    QColor m_labelColor;     // 标注颜色（馈线损耗/器件dB）
    QColor m_powerColor;     // 功率标注颜色
    QColor m_wallColor;      // 墙体颜色
    QColor m_textColor;      // 文字颜色
};

} // namespace Zhifen

#endif // DEVICE_COLOR_CONFIG_H
