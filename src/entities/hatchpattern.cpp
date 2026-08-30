#include "hatchpattern.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>
#include <QPainterPathStroker>

namespace Zhifen {

// Hatch::generatePath
QPainterPath Hatch::generatePath(const QRectF &bounds) const
{
    QPainterPath result;
    if (patternName == "SOLID") {
        // 实心填充
        for (auto &poly : boundaries) {
            QPainterPath path;
            path.addPolygon(poly);
            result.addPath(path);
        }
        return result;
    }

    // 获取图案定义
    HatchPatternLibrary &lib = HatchPatternLibrary::instance();
    HatchPatternDef *def = lib.pattern(patternName);
    if (!def || def->lines.isEmpty()) return result;

    // 对每条图案线生成填充
    for (auto &hatchLine : def->lines) {
        qreal angleRad = (hatchLine.angle + angle) * M_PI / 180.0;
        qreal cosA = qCos(angleRad);
        qreal sinA = qSin(angleRad);

        // 计算线的方向和间距
        QPointF dir(cosA, sinA);
        QPointF normal(-sinA, cosA);
        qreal spacing = qMax(qAbs(hatchLine.delta.y()), 1.0) * scale;

        // 计算覆盖范围需要的线条数
        qreal minProj = 1e9, maxProj = -1e9;
        for (auto &poly : boundaries) {
            for (auto &p : poly) {
                qreal proj = QPointF::dotProduct(p, normal);
                minProj = qMin(minProj, proj);
                maxProj = qMax(maxProj, proj);
            }
        }

        qreal startProj = minProj - spacing;
        qreal endProj = maxProj + spacing;

        for (qreal proj = startProj; proj <= endProj; proj += spacing) {
            QPointF lineOrigin = normal * proj;
            QPainterPath linePath;
            bool started = false;
            QPointF prevPoint;

            // 计算线与边界的交点
            QList<QPointF> intersections;
            for (auto &poly : boundaries) {
                for (int i = 0; i < poly.size(); i++) {
                    QPointF p1 = poly[i];
                    QPointF p2 = poly[(i + 1) % poly.size()];
                    qreal proj1 = QPointF::dotProduct(p1, normal);
                    qreal proj2 = QPointF::dotProduct(p2, normal);
                    if ((proj1 - proj) * (proj2 - proj) <= 0 && qAbs(proj2 - proj1) > 1e-6) {
                        qreal t = (proj - proj1) / (proj2 - proj1);
                        QPointF intersect = p1 + (p2 - p1) * t;
                        intersections.append(intersect);
                    }
                }
            }

            // 按沿方向排序
            std::sort(intersections.begin(), intersections.end(),
                      [dir](const QPointF &a, const QPointF &b) {
                          return QPointF::dotProduct(a, dir) < QPointF::dotProduct(b, dir);
                      });

            // 成对连接
            for (int i = 0; i + 1 < intersections.size(); i += 2) {
                linePath.moveTo(intersections[i]);
                linePath.lineTo(intersections[i + 1]);
            }

            result.addPath(linePath);
        }
    }

    return result;
}

// HatchPatternLibrary
HatchPatternLibrary::HatchPatternLibrary()
{
    resetToDefaults();
}

HatchPatternLibrary& HatchPatternLibrary::instance()
{
    static HatchPatternLibrary inst;
    return inst;
}

void HatchPatternLibrary::resetToDefaults()
{
    m_patterns.clear();
    m_categories.clear();
    initDefaults();
}

void HatchPatternLibrary::initDefaults()
{
    addSolid();
    addANSI31();
    addANSI32();
    addANSI33();
    addANSI34();
    addANSI35();
    addANSI36();
    addANSI37();
    addANSI38();
    addHoneycomb();
    addDots();
    addCross();
    addLines();
    addBrick();
    addConcrete();
    addSteel();
    addInsulation();
    addGravel();
    addSand();
    addWater();
    addGrass();
    addEarth();
    addRock();
    addWood();
    addGlass();
}

void HatchPatternLibrary::addSolid()
{
    HatchPatternDef def;
    def.name = "SOLID";
    def.description = "实心填充";
    def.type = Hatch_Predefined;
    m_patterns["SOLID"] = def;
    m_categories["实心"].append("SOLID");
}

void HatchPatternLibrary::addANSI31()
{
    HatchPatternDef def;
    def.name = "ANSI31";
    def.description = "ANSI铁、砖和石";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 45;
    line.origin = QPointF(0, 0);
    line.delta = QPointF(0, 4.7625);
    def.lines.append(line);
    m_patterns["ANSI31"] = def;
    m_categories["ANSI"].append("ANSI31");
}

void HatchPatternLibrary::addANSI32()
{
    HatchPatternDef def;
    def.name = "ANSI32";
    def.description = "ANSI钢";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 9.525);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 9.525);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["ANSI32"] = def;
    m_categories["ANSI"].append("ANSI32");
}

void HatchPatternLibrary::addANSI33()
{
    HatchPatternDef def;
    def.name = "ANSI33";
    def.description = "ANSI青铜、黄铜和紫铜";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 4.7625);
    line2.angle = 45; line2.origin = QPointF(3.375, 0); line2.delta = QPointF(0, 4.7625);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["ANSI33"] = def;
    m_categories["ANSI"].append("ANSI33");
}

void HatchPatternLibrary::addANSI34()
{
    HatchPatternDef def;
    def.name = "ANSI34";
    def.description = "ANSI塑料、橡胶和纤维";
    def.type = Hatch_Predefined;
    HatchLine line1, line2, line3;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 1.5875);
    line2.angle = 45; line2.origin = QPointF(1.125, 0); line2.delta = QPointF(0, 1.5875);
    line3.angle = 45; line3.origin = QPointF(2.25, 0); line3.delta = QPointF(0, 1.5875);
    def.lines.append(line1); def.lines.append(line2); def.lines.append(line3);
    m_patterns["ANSI34"] = def;
    m_categories["ANSI"].append("ANSI34");
}

void HatchPatternLibrary::addANSI35()
{
    HatchPatternDef def;
    def.name = "ANSI35";
    def.description = "ANSI耐火砖和保温材料";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 6.35);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 6.35);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["ANSI35"] = def;
    m_categories["ANSI"].append("ANSI35");
}

void HatchPatternLibrary::addANSI36()
{
    HatchPatternDef def;
    def.name = "ANSI36";
    def.description = "ANSI大理石、板岩和玻璃";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 45; line.origin = QPointF(0, 0); line.delta = QPointF(0, 3.175);
    def.lines.append(line);
    m_patterns["ANSI36"] = def;
    m_categories["ANSI"].append("ANSI36");
}

void HatchPatternLibrary::addANSI37()
{
    HatchPatternDef def;
    def.name = "ANSI37";
    def.description = "ANSI铅、锌、镁和声/热绝缘";
    def.type = Hatch_Predefined;
    HatchLine line1, line2, line3, line4;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 3.175);
    line2.angle = 45; line2.origin = QPointF(2.25, 0); line2.delta = QPointF(0, 3.175);
    line3.angle = 135; line3.origin = QPointF(0, 0); line3.delta = QPointF(0, 3.175);
    line4.angle = 135; line4.origin = QPointF(2.25, 0); line4.delta = QPointF(0, 3.175);
    def.lines.append(line1); def.lines.append(line2); def.lines.append(line3); def.lines.append(line4);
    m_patterns["ANSI37"] = def;
    m_categories["ANSI"].append("ANSI37");
}

void HatchPatternLibrary::addANSI38()
{
    HatchPatternDef def;
    def.name = "ANSI38";
    def.description = "ANSI铝";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 2.38125);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 2.38125);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["ANSI38"] = def;
    m_categories["ANSI"].append("ANSI38");
}

void HatchPatternLibrary::addHoneycomb()
{
    HatchPatternDef def;
    def.name = "HONEY";
    def.description = "蜂窝";
    def.type = Hatch_Predefined;
    HatchLine line1, line2, line3;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 5.196);
    line2.angle = 60; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 5.196);
    line3.angle = 120; line3.origin = QPointF(3, 0); line3.delta = QPointF(0, 5.196);
    def.lines.append(line1); def.lines.append(line2); def.lines.append(line3);
    m_patterns["HONEY"] = def;
    m_categories["其他"].append("HONEY");
}

void HatchPatternLibrary::addDots()
{
    HatchPatternDef def;
    def.name = "DOTS";
    def.description = "点";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 45; line.origin = QPointF(0, 0); line.delta = QPointF(1.7678, 1.7678);
    line.dashes = {0, 3.5355};
    def.lines.append(line);
    m_patterns["DOTS"] = def;
    m_categories["其他"].append("DOTS");
}

void HatchPatternLibrary::addCross()
{
    HatchPatternDef def;
    def.name = "CROSS";
    def.description = "十字";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 5);
    line2.angle = 90; line2.origin = QPointF(0, 0); line2.delta = QPointF(5, 0);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["CROSS"] = def;
    m_categories["其他"].append("CROSS");
}

void HatchPatternLibrary::addLines()
{
    HatchPatternDef def;
    def.name = "LINE";
    def.description = "水平线";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 0; line.origin = QPointF(0, 0); line.delta = QPointF(0, 5);
    def.lines.append(line);
    m_patterns["LINE"] = def;
    m_categories["其他"].append("LINE");
}

void HatchPatternLibrary::addBrick()
{
    HatchPatternDef def;
    def.name = "BRICK";
    def.description = "砖";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 10);
    line2.angle = 90; line2.origin = QPointF(0, 0); line2.delta = QPointF(10, 10);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["BRICK"] = def;
    m_categories["建筑"].append("BRICK");
}

void HatchPatternLibrary::addConcrete()
{
    HatchPatternDef def;
    def.name = "CONCRETE";
    def.description = "混凝土";
    def.type = Hatch_Predefined;
    HatchLine line1, line2, line3;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 8);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 8);
    line3.angle = 0; line3.origin = QPointF(2, 4); line3.delta = QPointF(0, 8);
    def.lines.append(line1); def.lines.append(line2); def.lines.append(line3);
    m_patterns["CONCRETE"] = def;
    m_categories["建筑"].append("CONCRETE");
}

void HatchPatternLibrary::addSteel()
{
    HatchPatternDef def;
    def.name = "STEEL";
    def.description = "钢";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 3);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 3);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["STEEL"] = def;
    m_categories["金属"].append("STEEL");
}

void HatchPatternLibrary::addInsulation()
{
    HatchPatternDef def;
    def.name = "INSUL";
    def.description = "保温";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 0; line.origin = QPointF(0, 0); line.delta = QPointF(0, 6);
    line.dashes = {2, 2, 0.5, 2};
    def.lines.append(line);
    m_patterns["INSUL"] = def;
    m_categories["建筑"].append("INSUL");
}

void HatchPatternLibrary::addGravel()
{
    HatchPatternDef def;
    def.name = "GRAVEL";
    def.description = "砾石";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 6);
    line2.angle = 135; line2.origin = QPointF(3, 0); line2.delta = QPointF(0, 6);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["GRAVEL"] = def;
    m_categories["地质"].append("GRAVEL");
}

void HatchPatternLibrary::addSand()
{
    HatchPatternDef def;
    def.name = "SAND";
    def.description = "沙";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 30; line.origin = QPointF(0, 0); line.delta = QPointF(0, 4);
    line.dashes = {0.5, 3.5};
    def.lines.append(line);
    m_patterns["SAND"] = def;
    m_categories["地质"].append("SAND");
}

void HatchPatternLibrary::addWater()
{
    HatchPatternDef def;
    def.name = "WATER";
    def.description = "水";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 6);
    line1.dashes = {2, 1, 0.5, 1};
    line2.angle = 0; line2.origin = QPointF(1.75, 3); line2.delta = QPointF(0, 6);
    line2.dashes = {2, 1, 0.5, 1};
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["WATER"] = def;
    m_categories["地质"].append("WATER");
}

void HatchPatternLibrary::addGrass()
{
    HatchPatternDef def;
    def.name = "GRASS";
    def.description = "草地";
    def.type = Hatch_Predefined;
    HatchLine line;
    line.angle = 75; line.origin = QPointF(0, 0); line.delta = QPointF(0, 5);
    line.dashes = {3, 2};
    def.lines.append(line);
    m_patterns["GRASS"] = def;
    m_categories["地质"].append("GRASS");
}

void HatchPatternLibrary::addEarth()
{
    HatchPatternDef def;
    def.name = "EARTH";
    def.description = "土";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 8);
    line2.angle = 135; line2.origin = QPointF(0, 4); line2.delta = QPointF(0, 8);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["EARTH"] = def;
    m_categories["地质"].append("EARTH");
}

void HatchPatternLibrary::addRock()
{
    HatchPatternDef def;
    def.name = "ROCK";
    def.description = "岩石";
    def.type = Hatch_Predefined;
    HatchLine line1, line2, line3;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 10);
    line2.angle = 60; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 10);
    line3.angle = 120; line3.origin = QPointF(5, 0); line3.delta = QPointF(0, 10);
    def.lines.append(line1); def.lines.append(line2); def.lines.append(line3);
    m_patterns["ROCK"] = def;
    m_categories["地质"].append("ROCK");
}

void HatchPatternLibrary::addWood()
{
    HatchPatternDef def;
    def.name = "WOOD";
    def.description = "木";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 0; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 8);
    line2.angle = 90; line2.origin = QPointF(0, 0); line2.delta = QPointF(8, 8);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["WOOD"] = def;
    m_categories["建筑"].append("WOOD");
}

void HatchPatternLibrary::addGlass()
{
    HatchPatternDef def;
    def.name = "GLASS";
    def.description = "玻璃";
    def.type = Hatch_Predefined;
    HatchLine line1, line2;
    line1.angle = 45; line1.origin = QPointF(0, 0); line1.delta = QPointF(0, 3);
    line2.angle = 135; line2.origin = QPointF(0, 0); line2.delta = QPointF(0, 3);
    def.lines.append(line1); def.lines.append(line2);
    m_patterns["GLASS"] = def;
    m_categories["建筑"].append("GLASS");
}

HatchPatternDef* HatchPatternLibrary::pattern(const QString &name)
{
    if (m_patterns.contains(name)) {
        return &m_patterns[name];
    }
    return nullptr;
}

QStringList HatchPatternLibrary::allPatternNames() const
{
    return m_patterns.keys();
}

QList<HatchPatternDef> HatchPatternLibrary::allPatterns() const
{
    return m_patterns.values();
}

QStringList HatchPatternLibrary::categories() const
{
    return m_categories.keys();
}

QStringList HatchPatternLibrary::patternsByCategory(const QString &category) const
{
    return m_categories.value(category);
}

void HatchPatternLibrary::addPattern(const HatchPatternDef &pattern)
{
    m_patterns[pattern.name] = pattern;
}

bool HatchPatternLibrary::removePattern(const QString &name)
{
    return m_patterns.remove(name) > 0;
}

bool HatchPatternLibrary::importPatterns(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray patterns = root["patterns"].toArray();
    for (auto val : patterns) {
        QJsonObject obj = val.toObject();
        HatchPatternDef def;
        def.name = obj["name"].toString();
        def.description = obj["description"].toString();
        def.type = static_cast<HatchPatternType>(obj["type"].toInt());
        QJsonArray lines = obj["lines"].toArray();
        for (auto lineVal : lines) {
            QJsonObject lineObj = lineVal.toObject();
            HatchLine line;
            line.angle = lineObj["angle"].toDouble();
            line.origin = QPointF(lineObj["originX"].toDouble(), lineObj["originY"].toDouble());
            line.delta = QPointF(lineObj["deltaX"].toDouble(), lineObj["deltaY"].toDouble());
            def.lines.append(line);
        }
        m_patterns[def.name] = def;
    }
    return true;
}

bool HatchPatternLibrary::exportPatterns(const QString &filePath) const
{
    QJsonObject root;
    QJsonArray patterns;
    for (auto it = m_patterns.begin(); it != m_patterns.end(); ++it) {
        QJsonObject obj;
        obj["name"] = it.value().name;
        obj["description"] = it.value().description;
        obj["type"] = it.value().type;
        QJsonArray lines;
        for (auto &line : it.value().lines) {
            QJsonObject lineObj;
            lineObj["angle"] = line.angle;
            lineObj["originX"] = line.origin.x();
            lineObj["originY"] = line.origin.y();
            lineObj["deltaX"] = line.delta.x();
            lineObj["deltaY"] = line.delta.y();
            lines.append(lineObj);
        }
        obj["lines"] = lines;
        patterns.append(obj);
    }
    root["patterns"] = patterns;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

} // namespace Zhifen
