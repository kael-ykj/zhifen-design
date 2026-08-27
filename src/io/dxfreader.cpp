#include "dxfreader.h"
#include "cadscene.h"
#include "document.h"
#include "lineitem.h"
#include "circleitem.h"
#include "arcitem.h"
#include "polylineitem.h"
#include "textitem.h"
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
    processEntities(pairs);
    return true;
}

QList<DxfReader::DxfPair> DxfReader::parsePairs(const QString &text)
{
    QList<DxfPair> pairs;
    QStringList lines = text.split('\n');
    for (int i = 0; i < lines.size() - 1; i += 2) {
        bool ok;
        int code = lines[i].trimmed().toInt(&ok);
        if (ok) {
            pairs.append({code, lines[i+1].trimmed()});
        }
    }
    return pairs;
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
            // LWPOLYLINE顶点
            if (currentType == "LWPOLYLINE" && p.code == 10) {
                QPointF pt(p.value.toDouble(), 0);
                if (i + 1 < pairs.size() && pairs[i+1].code == 20) {
                    pt.setY(pairs[i+1].value.toDouble());
                    i++;
                }
                polyPoints.append(pt);
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
