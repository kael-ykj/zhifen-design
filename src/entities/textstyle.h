#ifndef TEXTSTYLE_H
#define TEXTSTYLE_H

#include <QString>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QList>

namespace Zhifen {

// 文字样式
struct TextStyle {
    QString name = "Standard";       // 样式名称
    QString fontName = "SimSun";     // 字体名称
    qreal height = 0.0;              // 高度(0=可变)
    qreal widthFactor = 1.0;         // 宽度因子
    qreal obliqueAngle = 0.0;        // 倾斜角度
    bool bold = false;               // 粗体
    bool italic = false;             // 斜体
    bool underline = false;          // 下划线
    bool overline = false;           // 上划线
    bool backwards = false;          // 反向
    bool upsideDown = false;         // 倒置
    bool vertical = false;           // 垂直
    QString bigFont = "";            // 大字体(中文SHX)
    QColor color = QColor(255, 255, 255); // 默认颜色

    // 转换为QFont
    QFont toFont(qreal textHeight = 2.5) const {
        QFont font(fontName);
        font.setPointSizeF(height > 0 ? height : textHeight);
        font.setBold(bold);
        font.setItalic(italic);
        font.setUnderline(underline);
        font.setStrikeOut(overline);
        font.setLetterSpacing(QFont::AbsoluteSpacing, widthFactor);
        return font;
    }

    // 从QFont转换
    void fromFont(const QFont &font) {
        fontName = font.family();
        height = font.pointSizeF();
        bold = font.bold();
        italic = font.italic();
        underline = font.underline();
        overline = font.strikeOut();
    }
};

// 文字样式管理器
class TextStyleManager
{
public:
    static TextStyleManager& instance();

    // 添加/删除样式
    void addStyle(const TextStyle &style);
    bool removeStyle(const QString &name);
    TextStyle* style(const QString &name);
    TextStyle* currentStyle();
    void setCurrentStyle(const QString &name);

    // 获取所有样式
    QStringList allStyleNames() const;
    QList<TextStyle> allStyles() const;

    // 重命名
    bool renameStyle(const QString &oldName, const QString &newName);

    // 导入/导出
    bool exportStyles(const QString &filePath) const;
    bool importStyles(const QString &filePath);

    // 重置为默认
    void resetToDefaults();

private:
    TextStyleManager();
    QMap<QString, TextStyle> m_styles;
    QString m_currentStyle = "Standard";
    void initDefaults();
};

} // namespace Zhifen

#endif // TEXTSTYLE_H
