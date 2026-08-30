#ifndef PLOTSTYLE_H
#define PLOTSTYLE_H

#include <QString>
#include <QColor>
#include <QMap>
#include <QList>

namespace Zhifen {

// 打印样式类型
enum PlotStyleType {
    PlotStyle_ColorDependent = 0,  // 颜色相关打印样式(CTB)
    PlotStyle_Named = 1            // 命名打印样式(STB)
};

// 线条样式
enum PlotLineStyle {
    PlotLineStyle_ByLayer = 0,
    PlotLineStyle_Continuous = 1,
    PlotLineStyle_Dashed = 2,
    PlotLineStyle_Dotted = 3,
    PlotLineStyle_DashDot = 4,
    PlotLineStyle_ShortDash = 5,
    PlotLineStyle_MediumDash = 6,
    PlotLineStyle_LongDash = 7
};

// 打印样式
struct PlotStyle {
    QString name;
    QColor color = QColor(0, 0, 0);       // 打印颜色
    bool dither = true;                    // 抖动
    int grayscale = 0;                     // 灰度(0-100, 0=不转换)
    int penNumber = 1;                     // 笔号(1-32)
    qreal virtualPen = 0;                  // 虚拟笔(0=自动)
    qreal lineWeight = 0.25;               // 线宽(mm)
    PlotLineStyle lineStyle = PlotLineStyle_Continuous; // 线型
    qreal lineStyleScale = 1.0;            // 线型比例
    bool adaptive = true;                  // 自适应
    int screening = 100;                   // 淡显(0-100)
    bool endStyle = 0;                     // 端点样式(0=圆形,1=方形,2=菱形)
    bool joinStyle = 0;                    // 连接样式(0=圆形,1=斜接,2=斜角)
    qreal fillStyle = 0;                   // 填充样式
};

// 颜色相关打印样式表(CTB)
struct ColorDependentPlotStyle {
    int colorIndex = 1;                    // ACI颜色索引(1-255)
    QColor displayColor;                   // 显示颜色
    PlotStyle plotStyle;                   // 打印样式
};

// 打印样式表
class PlotStyleTable
{
public:
    PlotStyleTable();

    // 类型
    PlotStyleType type() const { return m_type; }
    void setType(PlotStyleType type) { m_type = type; }

    // 名称和描述
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QString description() const { return m_description; }
    void setDescription(const QString &desc) { m_description = desc; }

    // 颜色相关样式(CTB)
    PlotStyle* colorStyle(int colorIndex);
    void setColorStyle(int colorIndex, const PlotStyle &style);
    QList<ColorDependentPlotStyle> allColorStyles() const;

    // 命名样式(STB)
    void addNamedStyle(const PlotStyle &style);
    bool removeNamedStyle(const QString &name);
    PlotStyle* namedStyle(const QString &name);
    QStringList allNamedStyleNames() const;
    QList<PlotStyle> allNamedStyles() const;

    // 默认线宽
    qreal defaultLineWeight() const { return m_defaultLineWeight; }
    void setDefaultLineWeight(qreal w) { m_defaultLineWeight = w; }

    // 全局比例
    qreal scaleFactor() const { return m_scaleFactor; }
    void setScaleFactor(qreal f) { m_scaleFactor = f; }

    // 导入/导出
    bool exportCTB(const QString &filePath) const;
    bool importCTB(const QString &filePath);
    bool exportSTB(const QString &filePath) const;
    bool importSTB(const QString &filePath);

    // 重置为默认
    void resetToDefaults();

    // ACI颜色转RGB
    static QColor aciToRgb(int colorIndex);
    static int rgbToAci(const QColor &color);

private:
    PlotStyleType m_type = PlotStyle_ColorDependent;
    QString m_name = "monochrome.ctb";
    QString m_description;
    QMap<int, PlotStyle> m_colorStyles;      // CTB: 颜色索引->样式
    QMap<QString, PlotStyle> m_namedStyles;  // STB: 名称->样式
    qreal m_defaultLineWeight = 0.25;
    qreal m_scaleFactor = 1.0;

    void initDefaultColorStyles();
};

// 打印样式表管理器
class PlotStyleManager
{
public:
    static PlotStyleManager& instance();

    // 样式表管理
    void addStyleTable(const PlotStyleTable &table);
    bool removeStyleTable(const QString &name);
    PlotStyleTable* styleTable(const QString &name);
    PlotStyleTable* currentTable();
    void setCurrentTable(const QString &name);

    QStringList allTableNames() const;
    QList<PlotStyleTable> allTables() const;

    // 重置默认
    void resetToDefaults();

private:
    PlotStyleManager();
    QMap<QString, PlotStyleTable> m_tables;
    QString m_currentTable;
    void initDefaults();
};

} // namespace Zhifen

#endif // PLOTSTYLE_H
