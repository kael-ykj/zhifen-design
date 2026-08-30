#ifndef HATCHPATTERN_H
#define HATCHPATTERN_H

#include <QString>
#include <QColor>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QPolygonF>
#include <QPainterPath>

namespace Zhifen {

// 填充图案类型
enum HatchPatternType {
    Hatch_Predefined = 0,   // 预定义
    Hatch_UserDefined = 1,  // 用户定义
    Hatch_Custom = 2        // 自定义
};

// 填充样式
enum HatchStyle {
    HatchStyle_Normal = 0,     // 普通
    HatchStyle_Outer = 1,      // 外层
    HatchStyle_Ignore = 2      // 忽略
};

// 图案线定义
struct HatchLine {
    qreal angle = 0;           // 角度(度)
    QPointF origin;            // 起点
    QPointF delta;             // 偏移
    QList<qreal> dashes;       // 虚线模式
};

// 填充图案定义
struct HatchPatternDef {
    QString name;              // 图案名称
    QString description;       // 描述
    HatchPatternType type = Hatch_Predefined;
    QList<HatchLine> lines;    // 图案线
};

// 填充对象
struct Hatch {
    QString patternName = "ANSI31";  // 图案名称
    HatchPatternType patternType = Hatch_Predefined;
    HatchStyle style = HatchStyle_Normal;
    QColor color = QColor(255, 255, 255);  // 填充颜色
    qreal scale = 1.0;             // 比例
    qreal angle = 0;               // 角度
    qreal lineWeight = 0.18;       // 线宽
    bool associative = true;       // 关联
    bool annotative = false;       // 注释性
    double transparency = 0;       // 透明度
    QList<QPolygonF> boundaries;   // 边界(可以是多个)
    QPainterPath path;             // 填充路径

    // 生成填充路径
    QPainterPath generatePath(const QRectF &bounds) const;
};

// 图案库
class HatchPatternLibrary
{
public:
    static HatchPatternLibrary& instance();

    // 获取图案
    HatchPatternDef* pattern(const QString &name);
    QStringList allPatternNames() const;
    QList<HatchPatternDef> allPatterns() const;

    // 按类别获取
    QStringList categories() const;
    QStringList patternsByCategory(const QString &category) const;

    // 添加/删除图案
    void addPattern(const HatchPatternDef &pattern);
    bool removePattern(const QString &name);

    // 导入/导出
    bool importPatterns(const QString &filePath);
    bool exportPatterns(const QString &filePath) const;

    // 重置默认
    void resetToDefaults();

private:
    HatchPatternLibrary();
    QMap<QString, HatchPatternDef> m_patterns;
    QMap<QString, QStringList> m_categories;
    void initDefaults();
    void addANSI31();
    void addANSI32();
    void addANSI33();
    void addANSI34();
    void addANSI35();
    void addANSI36();
    void addANSI37();
    void addANSI38();
    void addSolid();
    void addHoneycomb();
    void addDots();
    void addCross();
    void addLines();
    void addBrick();
    void addConcrete();
    void addSteel();
    void addInsulation();
    void addGravel();
    void addSand();
    void addWater();
    void addGrass();
    void addEarth();
    void addRock();
    void addWood();
    void addGlass();
};

} // namespace Zhifen

#endif // HATCHPATTERN_H
