#include "dimensionstylemanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

DimensionStyleManager::DimensionStyleManager()
{
    initDefaults();
}

DimensionStyleManager& DimensionStyleManager::instance()
{
    static DimensionStyleManager inst;
    return inst;
}

void DimensionStyleManager::initDefaults()
{
    // Standard样式
    DimensionStyle standard;
    standard.name = "Standard";
    m_styles["Standard"] = standard;

    // 建筑标注样式
    DimensionStyle architectural;
    architectural.name = "Architectural";
    architectural.arrowType = Arrow_ArchTick;
    architectural.firstArrow = Arrow_ArchTick;
    architectural.secondArrow = Arrow_ArchTick;
    architectural.arrowSize = 2.0;
    architectural.textHeight = 2.5;
    architectural.extLineOffset = 0.625;
    architectural.extLineExtend = 1.25;
    architectural.textOffset = 0.625;
    architectural.linearPrecision = 0;
    architectural.dimLineColor = QColor(255, 255, 255);
    architectural.extLineColor = QColor(255, 255, 255);
    architectural.textColor = QColor(255, 255, 255);
    m_styles["Architectural"] = architectural;

    // 机械标注样式
    DimensionStyle mechanical;
    mechanical.name = "Mechanical";
    mechanical.arrowType = Arrow_ClosedFilled;
    mechanical.arrowSize = 2.5;
    mechanical.textHeight = 2.5;
    mechanical.extLineOffset = 0.625;
    mechanical.extLineExtend = 1.25;
    mechanical.textOffset = 0.625;
    mechanical.linearPrecision = 2;
    mechanical.textAlignment = TextAlign_Aligned;
    mechanical.dimLineColor = QColor(255, 255, 255);
    mechanical.extLineColor = QColor(255, 255, 255);
    mechanical.textColor = QColor(255, 255, 255);
    m_styles["Mechanical"] = mechanical;

    // 室分设计标注样式
    DimensionStyle indoor;
    indoor.name = "IndoorDesign";
    indoor.arrowType = Arrow_ClosedFilled;
    indoor.arrowSize = 3.0;
    indoor.textHeight = 3.0;
    indoor.extLineOffset = 1.0;
    indoor.extLineExtend = 1.5;
    indoor.textOffset = 1.0;
    indoor.linearPrecision = 0;
    indoor.textStyle = "Dimension";
    indoor.dimLineColor = QColor(0, 255, 0);
    indoor.extLineColor = QColor(0, 255, 0);
    indoor.textColor = QColor(0, 255, 0);
    indoor.scaleFactor = 100.0;
    m_styles["IndoorDesign"] = indoor;

    // 大比例标注样式
    DimensionStyle largeScale;
    largeScale.name = "LargeScale";
    largeScale.arrowType = Arrow_ClosedFilled;
    largeScale.arrowSize = 5.0;
    largeScale.textHeight = 5.0;
    largeScale.extLineOffset = 2.0;
    largeScale.extLineExtend = 3.0;
    largeScale.textOffset = 2.0;
    largeScale.linearPrecision = 0;
    largeScale.scaleFactor = 200.0;
    m_styles["LargeScale"] = largeScale;

    // 小比例标注样式
    DimensionStyle smallScale;
    smallScale.name = "SmallScale";
    smallScale.arrowType = Arrow_ClosedFilled;
    smallScale.arrowSize = 1.5;
    smallScale.textHeight = 1.5;
    smallScale.extLineOffset = 0.5;
    smallScale.extLineExtend = 0.75;
    smallScale.textOffset = 0.5;
    smallScale.linearPrecision = 1;
    smallScale.scaleFactor = 50.0;
    m_styles["SmallScale"] = smallScale;

    // 公差标注样式
    DimensionStyle tolerance;
    tolerance.name = "Tolerance";
    tolerance.arrowType = Arrow_ClosedFilled;
    tolerance.arrowSize = 2.5;
    tolerance.textHeight = 2.5;
    tolerance.toleranceEnabled = true;
    tolerance.toleranceType = 2; // 极限偏差
    tolerance.toleranceUpper = 0.05;
    tolerance.toleranceLower = 0.02;
    tolerance.tolerancePrecision = 2;
    tolerance.linearPrecision = 2;
    m_styles["Tolerance"] = tolerance;
}

void DimensionStyleManager::addStyle(const DimensionStyle &style)
{
    m_styles[style.name] = style;
}

bool DimensionStyleManager::removeStyle(const QString &name)
{
    if (name == "Standard") return false;
    return m_styles.remove(name) > 0;
}

DimensionStyle* DimensionStyleManager::style(const QString &name)
{
    if (m_styles.contains(name)) {
        return &m_styles[name];
    }
    return nullptr;
}

DimensionStyle* DimensionStyleManager::currentStyle()
{
    if (m_styles.contains(m_currentStyle)) {
        return &m_styles[m_currentStyle];
    }
    return &m_styles["Standard"];
}

void DimensionStyleManager::setCurrentStyle(const QString &name)
{
    if (m_styles.contains(name)) {
        m_currentStyle = name;
    }
}

QStringList DimensionStyleManager::allStyleNames() const
{
    return m_styles.keys();
}

QList<DimensionStyle> DimensionStyleManager::allStyles() const
{
    return m_styles.values();
}

bool DimensionStyleManager::renameStyle(const QString &oldName, const QString &newName)
{
    if (!m_styles.contains(oldName) || m_styles.contains(newName)) return false;
    if (oldName == "Standard") return false;

    DimensionStyle style = m_styles[oldName];
    style.name = newName;
    m_styles.remove(oldName);
    m_styles[newName] = style;

    if (m_currentStyle == oldName) {
        m_currentStyle = newName;
    }
    return true;
}

bool DimensionStyleManager::copyStyle(const QString &sourceName, const QString &newName)
{
    if (!m_styles.contains(sourceName) || m_styles.contains(newName)) return false;
    DimensionStyle style = m_styles[sourceName];
    style.name = newName;
    m_styles[newName] = style;
    return true;
}

bool DimensionStyleManager::exportStyles(const QString &filePath) const
{
    QJsonObject root;
    QJsonArray styles;
    for (auto it = m_styles.begin(); it != m_styles.end(); ++it) {
        const DimensionStyle &s = it.value();
        QJsonObject obj;
        obj["name"] = s.name;
        obj["textHeight"] = s.textHeight;
        obj["arrowSize"] = s.arrowSize;
        obj["arrowType"] = s.arrowType;
        obj["extLineOffset"] = s.extLineOffset;
        obj["extLineExtend"] = s.extLineExtend;
        obj["linearPrecision"] = s.linearPrecision;
        obj["scaleFactor"] = s.scaleFactor;
        obj["textStyle"] = s.textStyle;
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

bool DimensionStyleManager::importStyles(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray styles = root["styles"].toArray();
    for (auto val : styles) {
        QJsonObject obj = val.toObject();
        DimensionStyle s;
        s.name = obj["name"].toString();
        s.textHeight = obj["textHeight"].toDouble();
        s.arrowSize = obj["arrowSize"].toDouble();
        s.arrowType = static_cast<ArrowType>(obj["arrowType"].toInt());
        s.extLineOffset = obj["extLineOffset"].toDouble();
        s.extLineExtend = obj["extLineExtend"].toDouble();
        s.linearPrecision = obj["linearPrecision"].toInt();
        s.scaleFactor = obj["scaleFactor"].toDouble();
        s.textStyle = obj["textStyle"].toString();
        m_styles[s.name] = s;
    }
    m_currentStyle = root["current"].toString("Standard");
    return true;
}

void DimensionStyleManager::resetToDefaults()
{
    m_styles.clear();
    initDefaults();
    m_currentStyle = "Standard";
}

} // namespace Zhifen
