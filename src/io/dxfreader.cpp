#include "dxfreader.h"
#include "cadscene.h"
#include "cad/document.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/arcitem.h"
#include "entities/polylineitem.h"
#include "entities/rectangleitem.h"
#include "entities/textitem.h"
#include "entities/dimensionitem.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>

DxfReader::DxfReader(CadScene *scene, Document *doc)
    : m_scene(scene), m_document(doc)
{
}

bool DxfReader::read(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_error = "无法打开文件: " + filePath;
        return false;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    file.close();
    return readFromText(text);
}

bool DxfReader::readFromText(const QString &text)
{
    QList<DxfPair> pairs = parsePairs(text);
    processTables(pairs);
    processEntities(pairs);
    return true;
}

QList<DxfReader::DxfPair> DxfReader::parsePairs(const QString &text)
{
    QList<DxfPair> pairs;
    QStringList lines = text.split('\n');
    for (int i = 0; i + 1 < lines.size(); i += 2) {
        bool ok;
        int code = lines[i].trimmed().toInt(&ok);
        if (ok) {
            pairs.append({code, lines[i+1].trimmed()});
        }
    }
    return pairs;
}

void DxfReader::processTables(const QList<DxfPair> &pairs)
{
    if (!m_document) return;
    bool inTables = false;
    bool inLayerTable = false;
    QString layerName;
    int layerColor = 7;

    for (int i = 0; i < pairs.size(); i++) {
        const DxfPair &p = pairs[i];
        if (p.code == 2 && p.value == "TABLES") { inTables = true; continue; }
        if (!inTables) continue;
        if (p.code == 0 && p.value == "ENDSEC") break;
        if (p.code == 0 && p.value == "TABLE") {
            if (i+1 < pairs.size() && pairs[i+1].code == 2 && pairs[i+1].value == "LAYER")
                inLayerTable = true;
            continue;
        }
        if (p.code == 0 && p.value == "ENDTAB") { inLayerTable = false; continue; }
        if (!inLayerTable) continue;
        if (p.code == 0 && p.value == "LAYER") {
            if (!layerName.isEmpty()) m_document->addLayer(layerName, QColor::fromHsl((layerColor-1)*30, 200, 128));
            layerName = ""; layerColor = 7;
            continue;
        }
        if (p.code == 2) layerName = p.value;
        if (p.code == 62) layerColor = p.value.toInt();
    }
    if (!layerName.isEmpty()) m_document->addLayer(layerName, QColor::fromHsl((layerColor-1)*30, 200, 128));
}

void DxfReader::processEntities(const QList<DxfPair> &pairs)
{
    bool inEntities = false;
    QString currentType;
    QPointF p1, p2, center;
    qreal radius = 0, startAngle = 0, endAngle = 0;
    QString layer = "0";
    QString textContent;
    qreal textHeight = 2.5;
    QPolygonF polyPoints;
    bool polyClosed = false;

    for (int i = 0; i < pairs.size(); i++) {
        const DxfPair &p = pairs[i];
        if (p.code == 2 && p.value == "ENTITIES") { inEntities = true; continue; }
        if (!inEntities) continue;
        if (p.code == 0 && p.value == "ENDSEC") break;

        if (p.code == 0) {
            // 保存上一个实体
            if (currentType == "LINE" && m_scene) {
                m_scene->addItem(new LineItem(p1, p2));
            } else if (currentType == "CIRCLE" && m_scene) {
                m_scene->addItem(new CircleItem(center, radius));
            } else if (currentType == "ARC" && m_scene) {
                m_scene->addItem(new ArcItem(center, radius, startAngle, endAngle - startAngle));
            } else if (currentType == "TEXT" && m_scene) {
                m_scene->addItem(new TextItem(p1, textContent, textHeight));
            } else if (currentType == "LWPOLYLINE" && m_scene && polyPoints.size() >= 2) {
                m_scene->addItem(new PolylineItem(polyPoints, polyClosed));
            }
            // 重置
            currentType = p.value;
            p1 = p2 = center = QPointF();
            radius = 0; startAngle = 0; endAngle = 0;
            layer = "0"; textContent = ""; textHeight = 2.5;
            polyPoints.clear(); polyClosed = false;
            continue;
        }

        if (!currentType.isEmpty()) {
            if (p.code == 8) layer = p.value;
            if (p.code == 10) p1.setX(p.value.toDouble());
            if (p.code == 20) p1.setY(p.value.toDouble());
            if (p.code == 11) p2.setX(p.value.toDouble());
            if (p.code == 21) p2.setY(p.value.toDouble());
            if (p.code == 40) { radius = p.value.toDouble(); textHeight = p.value.toDouble(); }
            if (p.code == 50) startAngle = p.value.toDouble();
            if (p.code == 51) endAngle = p.value.toDouble();
            if (p.code == 1) textContent = p.value;
            if (p.code == 70) polyClosed = (p.value.toInt() & 1) != 0;

            // LWPOLYLINE顶点 - 修复：只在LWPOLYLINE类型下处理，不修改p1
            if (currentType == "LWPOLYLINE" && p.code == 10) {
                QPointF pt(p.value.toDouble(), 0);
                if (i + 1 < pairs.size() && pairs[i+1].code == 20) {
                    pt.setY(pairs[i+1].value.toDouble());
                    i++;
                }
                polyPoints.append(pt);
            }
            // CIRCLE/ARC圆心
            if ((currentType == "CIRCLE" || currentType == "ARC") && p.code == 10) {
                center.setX(p.value.toDouble());
            }
            if ((currentType == "CIRCLE" || currentType == "ARC") && p.code == 20) {
                center.setY(p.value.toDouble());
            }
        }
    }
    // 保存最后一个实体
    if (currentType == "LINE" && m_scene) m_scene->addItem(new LineItem(p1, p2));
    else if (currentType == "CIRCLE" && m_scene) m_scene->addItem(new CircleItem(center, radius));
    else if (currentType == "ARC" && m_scene) m_scene->addItem(new ArcItem(center, radius, startAngle, endAngle - startAngle));
    else if (currentType == "TEXT" && m_scene) m_scene->addItem(new TextItem(p1, textContent, textHeight));
    else if (currentType == "LWPOLYLINE" && m_scene && polyPoints.size() >= 2) m_scene->addItem(new PolylineItem(polyPoints, polyClosed));
}
