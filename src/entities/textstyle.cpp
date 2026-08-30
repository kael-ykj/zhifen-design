#include "textstyle.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

TextStyleManager::TextStyleManager()
{
    initDefaults();
}

TextStyleManager& TextStyleManager::instance()
{
    static TextStyleManager inst;
    return inst;
}

void TextStyleManager::initDefaults()
{
    // Standard样式
    TextStyle standard;
    standard.name = "Standard";
    standard.fontName = "SimSun";
    standard.height = 0;
    standard.widthFactor = 1.0;
    m_styles["Standard"] = standard;

    // 工程字样式
    TextStyle engineering;
    engineering.name = "Engineering";
    engineering.fontName = "SimHei";
    engineering.height = 0;
    engineering.widthFactor = 0.7;
    m_styles["Engineering"] = engineering;

    // 标注文字样式
    TextStyle dimension;
    dimension.name = "Dimension";
    dimension.fontName = "SimSun";
    dimension.height = 2.5;
    dimension.widthFactor = 1.0;
    m_styles["Dimension"] = dimension;

    // 标题文字样式
    TextStyle title;
    title.name = "Title";
    title.fontName = "SimHei";
    title.height = 5.0;
    title.bold = true;
    title.widthFactor = 1.0;
    m_styles["Title"] = title;

    // 注释文字样式
    TextStyle note;
    note.name = "Note";
    note.fontName = "SimSun";
    note.height = 3.0;
    note.widthFactor = 1.0;
    m_styles["Note"] = note;

    // 中文大字体样式
    TextStyle chinese;
    chinese.name = "Chinese";
    chinese.fontName = "Microsoft YaHei";
    chinese.height = 0;
    chinese.widthFactor = 1.0;
    chinese.bigFont = "gbcbig.shx";
    m_styles["Chinese"] = chinese;

    // 英文样式
    TextStyle english;
    english.name = "English";
    english.fontName = "Arial";
    english.height = 0;
    english.widthFactor = 1.0;
    m_styles["English"] = english;

    // 技术要求样式
    TextStyle tech;
    tech.name = "TechReq";
    tech.fontName = "SimSun";
    tech.height = 3.5;
    tech.widthFactor = 0.8;
    m_styles["TechReq"] = tech;
}

void TextStyleManager::addStyle(const TextStyle &style)
{
    m_styles[style.name] = style;
}

bool TextStyleManager::removeStyle(const QString &name)
{
    if (name == "Standard") return false; // 不能删除Standard
    return m_styles.remove(name) > 0;
}

TextStyle* TextStyleManager::style(const QString &name)
{
    if (m_styles.contains(name)) {
        return &m_styles[name];
    }
    return nullptr;
}

TextStyle* TextStyleManager::currentStyle()
{
    if (m_styles.contains(m_currentStyle)) {
        return &m_styles[m_currentStyle];
    }
    return &m_styles["Standard"];
}

void TextStyleManager::setCurrentStyle(const QString &name)
{
    if (m_styles.contains(name)) {
        m_currentStyle = name;
    }
}

QStringList TextStyleManager::allStyleNames() const
{
    return m_styles.keys();
}

QList<TextStyle> TextStyleManager::allStyles() const
{
    return m_styles.values();
}

bool TextStyleManager::renameStyle(const QString &oldName, const QString &newName)
{
    if (!m_styles.contains(oldName) || m_styles.contains(newName)) return false;
    if (oldName == "Standard") return false;

    TextStyle style = m_styles[oldName];
    style.name = newName;
    m_styles.remove(oldName);
    m_styles[newName] = style;

    if (m_currentStyle == oldName) {
        m_currentStyle = newName;
    }
    return true;
}

bool TextStyleManager::exportStyles(const QString &filePath) const
{
    QJsonObject root;
    QJsonArray styles;
    for (auto it = m_styles.begin(); it != m_styles.end(); ++it) {
        const TextStyle &s = it.value();
        QJsonObject obj;
        obj["name"] = s.name;
        obj["fontName"] = s.fontName;
        obj["height"] = s.height;
        obj["widthFactor"] = s.widthFactor;
        obj["obliqueAngle"] = s.obliqueAngle;
        obj["bold"] = s.bold;
        obj["italic"] = s.italic;
        obj["underline"] = s.underline;
        obj["overline"] = s.overline;
        obj["backwards"] = s.backwards;
        obj["upsideDown"] = s.upsideDown;
        obj["vertical"] = s.vertical;
        obj["bigFont"] = s.bigFont;
        styles.append(obj);
    }
    root["styles"] = styles;
    root["current"] = m_currentStyle;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool TextStyleManager::importStyles(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray styles = root["styles"].toArray();
    for (auto val : styles) {
        QJsonObject obj = val.toObject();
        TextStyle s;
        s.name = obj["name"].toString();
        s.fontName = obj["fontName"].toString();
        s.height = obj["height"].toDouble();
        s.widthFactor = obj["widthFactor"].toDouble();
        s.obliqueAngle = obj["obliqueAngle"].toDouble();
        s.bold = obj["bold"].toBool();
        s.italic = obj["italic"].toBool();
        s.underline = obj["underline"].toBool();
        s.overline = obj["overline"].toBool();
        s.backwards = obj["backwards"].toBool();
        s.upsideDown = obj["upsideDown"].toBool();
        s.vertical = obj["vertical"].toBool();
        s.bigFont = obj["bigFont"].toString();
        m_styles[s.name] = s;
    }
    m_currentStyle = root["current"].toString("Standard");
    return true;
}

void TextStyleManager::resetToDefaults()
{
    m_styles.clear();
    initDefaults();
    m_currentStyle = "Standard";
}

} // namespace Zhifen
