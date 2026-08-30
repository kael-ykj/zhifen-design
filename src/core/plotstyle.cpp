#include "plotstyle.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

namespace Zhifen {

PlotStyleTable::PlotStyleTable()
{
    initDefaultColorStyles();
}

void PlotStyleTable::initDefaultColorStyles()
{
    // 初始化255种ACI颜色的默认打印样式
    for (int i = 1; i <= 255; i++) {
        PlotStyle style;
        style.color = QColor(0, 0, 0); // 默认全部打印为黑色
        style.lineWeight = 0.25;
        style.screening = 100;
        m_colorStyles[i] = style;
    }

    // 设置一些常用颜色的线宽
    m_colorStyles[1].lineWeight = 0.5;   // 红色
    m_colorStyles[2].lineWeight = 0.35;  // 黄色
    m_colorStyles[3].lineWeight = 0.35;  // 绿色
    m_colorStyles[4].lineWeight = 0.35;  // 青色
    m_colorStyles[5].lineWeight = 0.5;   // 蓝色
    m_colorStyles[6].lineWeight = 0.35;  // 品红
    m_colorStyles[7].lineWeight = 0.25;  // 白色/黑色
    m_colorStyles[8].lineWeight = 0.25;  // 灰色
    m_colorStyles[9].lineWeight = 0.18;  // 浅灰
}

PlotStyle* PlotStyleTable::colorStyle(int colorIndex)
{
    if (m_colorStyles.contains(colorIndex)) {
        return &m_colorStyles[colorIndex];
    }
    return nullptr;
}

void PlotStyleTable::setColorStyle(int colorIndex, const PlotStyle &style)
{
    m_colorStyles[colorIndex] = style;
}

QList<ColorDependentPlotStyle> PlotStyleTable::allColorStyles() const
{
    QList<ColorDependentPlotStyle> result;
    for (auto it = m_colorStyles.begin(); it != m_colorStyles.end(); ++it) {
        ColorDependentPlotStyle cps;
        cps.colorIndex = it.key();
        cps.displayColor = aciToRgb(it.key());
        cps.plotStyle = it.value();
        result.append(cps);
    }
    return result;
}

void PlotStyleTable::addNamedStyle(const PlotStyle &style)
{
    m_namedStyles[style.name] = style;
}

bool PlotStyleTable::removeNamedStyle(const QString &name)
{
    return m_namedStyles.remove(name) > 0;
}

PlotStyle* PlotStyleTable::namedStyle(const QString &name)
{
    if (m_namedStyles.contains(name)) {
        return &m_namedStyles[name];
    }
    return nullptr;
}

QStringList PlotStyleTable::allNamedStyleNames() const
{
    return m_namedStyles.keys();
}

QList<PlotStyle> PlotStyleTable::allNamedStyles() const
{
    return m_namedStyles.values();
}

bool PlotStyleTable::exportCTB(const QString &filePath) const
{
    QJsonObject root;
    root["name"] = m_name;
    root["description"] = m_description;
    root["type"] = "CTB";
    root["defaultLineWeight"] = m_defaultLineWeight;
    root["scaleFactor"] = m_scaleFactor;

    QJsonArray styles;
    for (auto it = m_colorStyles.begin(); it != m_colorStyles.end(); ++it) {
        QJsonObject obj;
        obj["colorIndex"] = it.key();
        obj["color"] = it.value().color.name();
        obj["lineWeight"] = it.value().lineWeight;
        obj["screening"] = it.value().screening;
        obj["lineStyle"] = it.value().lineStyle;
        styles.append(obj);
    }
    root["styles"] = styles;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool PlotStyleTable::importCTB(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    m_name = root["name"].toString();
    m_description = root["description"].toString();
    m_defaultLineWeight = root["defaultLineWeight"].toDouble();
    m_scaleFactor = root["scaleFactor"].toDouble();

    QJsonArray styles = root["styles"].toArray();
    for (auto val : styles) {
        QJsonObject obj = val.toObject();
        int idx = obj["colorIndex"].toInt();
        PlotStyle style;
        style.color = QColor(obj["color"].toString());
        style.lineWeight = obj["lineWeight"].toDouble();
        style.screening = obj["screening"].toInt();
        style.lineStyle = static_cast<PlotLineStyle>(obj["lineStyle"].toInt());
        m_colorStyles[idx] = style;
    }
    return true;
}

bool PlotStyleTable::exportSTB(const QString &filePath) const
{
    QJsonObject root;
    root["name"] = m_name;
    root["description"] = m_description;
    root["type"] = "STB";

    QJsonArray styles;
    for (auto it = m_namedStyles.begin(); it != m_namedStyles.end(); ++it) {
        QJsonObject obj;
        obj["name"] = it.value().name;
        obj["color"] = it.value().color.name();
        obj["lineWeight"] = it.value().lineWeight;
        obj["screening"] = it.value().screening;
        styles.append(obj);
    }
    root["styles"] = styles;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool PlotStyleTable::importSTB(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    m_name = root["name"].toString();
    m_description = root["description"].toString();

    QJsonArray styles = root["styles"].toArray();
    for (auto val : styles) {
        QJsonObject obj = val.toObject();
        PlotStyle style;
        style.name = obj["name"].toString();
        style.color = QColor(obj["color"].toString());
        style.lineWeight = obj["lineWeight"].toDouble();
        style.screening = obj["screening"].toInt();
        m_namedStyles[style.name] = style;
    }
    return true;
}

void PlotStyleTable::resetToDefaults()
{
    m_colorStyles.clear();
    m_namedStyles.clear();
    initDefaultColorStyles();
}

QColor PlotStyleTable::aciToRgb(int colorIndex)
{
    // AutoCAD ACI颜色表(简化版，前9种标准颜色)
    static const QColor standardColors[] = {
        QColor(255, 0, 0),    // 1 红
        QColor(255, 255, 0),  // 2 黄
        QColor(0, 255, 0),    // 3 绿
        QColor(0, 255, 255),  // 4 青
        QColor(0, 0, 255),    // 5 蓝
        QColor(255, 0, 255),  // 6 品红
        QColor(255, 255, 255),// 7 白/黑
        QColor(128, 128, 128),// 8 灰
        QColor(192, 192, 192) // 9 浅灰
    };

    if (colorIndex >= 1 && colorIndex <= 9) {
        return standardColors[colorIndex - 1];
    }

    // 10-249: 色轮颜色(简化)
    if (colorIndex >= 10 && colorIndex <= 249) {
        int hue = ((colorIndex - 10) % 24) * 15;
        QColor c;
        c.setHsv(hue, 255, 255);
        return c;
    }

    // 250-255: 灰度
    if (colorIndex >= 250 && colorIndex <= 255) {
        int gray = 255 - (colorIndex - 250) * 20;
        return QColor(gray, gray, gray);
    }

    return QColor(0, 0, 0);
}

int PlotStyleTable::rgbToAci(const QColor &color)
{
    // 简化：找到最接近的标准颜色
    static const QColor standardColors[] = {
        QColor(255, 0, 0), QColor(255, 255, 0), QColor(0, 255, 0),
        QColor(0, 255, 255), QColor(0, 0, 255), QColor(255, 0, 255),
        QColor(255, 255, 255), QColor(128, 128, 128), QColor(192, 192, 192)
    };

    int bestIdx = 7;
    int bestDist = 1000000;
    for (int i = 0; i < 9; i++) {
        int dr = color.red() - standardColors[i].red();
        int dg = color.green() - standardColors[i].green();
        int db = color.blue() - standardColors[i].blue();
        int dist = dr*dr + dg*dg + db*db;
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i + 1;
        }
    }
    return bestIdx;
}

// PlotStyleManager
PlotStyleManager::PlotStyleManager()
{
    initDefaults();
}

PlotStyleManager& PlotStyleManager::instance()
{
    static PlotStyleManager inst;
    return inst;
}

void PlotStyleManager::initDefaults()
{
    // 单色打印样式
    PlotStyleTable monochrome;
    monochrome.setName("monochrome.ctb");
    monochrome.setDescription("所有颜色打印为黑色");
    for (int i = 1; i <= 255; i++) {
        PlotStyle style;
        style.color = QColor(0, 0, 0);
        style.lineWeight = 0.25;
        monochrome.setColorStyle(i, style);
    }
    m_tables["monochrome.ctb"] = monochrome;

    // 灰度打印样式
    PlotStyleTable grayscale;
    grayscale.setName("grayscale.ctb");
    grayscale.setDescription("所有颜色按灰度打印");
    for (int i = 1; i <= 255; i++) {
        PlotStyle style;
        QColor c = PlotStyleTable::aciToRgb(i);
        int gray = qGray(c.rgb());
        style.color = QColor(gray, gray, gray);
        style.lineWeight = 0.25;
        grayscale.setColorStyle(i, style);
    }
    m_tables["grayscale.ctb"] = grayscale;

    // 彩色打印样式
    PlotStyleTable color;
    color.setName("color.ctb");
    color.setDescription("按显示颜色打印");
    for (int i = 1; i <= 255; i++) {
        PlotStyle style;
        style.color = PlotStyleTable::aciToRgb(i);
        style.lineWeight = 0.25;
        color.setColorStyle(i, style);
    }
    m_tables["color.ctb"] = color;

    // 室分设计打印样式
    PlotStyleTable indoor;
    indoor.setName("indoor_design.ctb");
    indoor.setDescription("室分设计专用打印样式");
    for (int i = 1; i <= 255; i++) {
        PlotStyle style;
        style.color = QColor(0, 0, 0);
        style.lineWeight = 0.25;
        indoor.setColorStyle(i, style);
    }
    // 室分专用线宽设置
    indoor.colorStyle(1)->lineWeight = 0.5;   // 红色-馈线
    indoor.colorStyle(3)->lineWeight = 0.35;  // 绿色-天线
    indoor.colorStyle(5)->lineWeight = 0.5;   // 蓝色-墙体
    indoor.colorStyle(7)->lineWeight = 0.18;  // 白色-标注
    m_tables["indoor_design.ctb"] = indoor;

    m_currentTable = "monochrome.ctb";
}

void PlotStyleManager::addStyleTable(const PlotStyleTable &table)
{
    m_tables[table.name()] = table;
}

bool PlotStyleManager::removeStyleTable(const QString &name)
{
    return m_tables.remove(name) > 0;
}

PlotStyleTable* PlotStyleManager::styleTable(const QString &name)
{
    if (m_tables.contains(name)) {
        return &m_tables[name];
    }
    return nullptr;
}

PlotStyleTable* PlotStyleManager::currentTable()
{
    if (m_tables.contains(m_currentTable)) {
        return &m_tables[m_currentTable];
    }
    return &m_tables.first();
}

void PlotStyleManager::setCurrentTable(const QString &name)
{
    if (m_tables.contains(name)) {
        m_currentTable = name;
    }
}

QStringList PlotStyleManager::allTableNames() const
{
    return m_tables.keys();
}

QList<PlotStyleTable> PlotStyleManager::allTables() const
{
    return m_tables.values();
}

void PlotStyleManager::resetToDefaults()
{
    m_tables.clear();
    initDefaults();
}

} // namespace Zhifen
