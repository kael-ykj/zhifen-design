#include "projectio.h"
#include "cad/cadscene.h"
#include "cad/document.h"
#include "entities/caditem.h"
#include "entities/lineitem.h"
#include "entities/circleitem.h"
#include "entities/arcitem.h"
#include "entities/polylineitem.h"
#include "entities/rectangleitem.h"
#include "entities/textitem.h"
#include "entities/dimensionitem.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

ProjectIO::ProjectIO(CadScene *scene, Document *doc)
    : m_scene(scene), m_document(doc)
{
}

bool ProjectIO::save(const QString &filePath)
{
    QJsonObject root;
    root["version"] = "3.1.0";
    root["app"] = "智分Design";

    // 文档属性
    QJsonObject docObj;
    docObj["name"] = m_document ? m_document->name() : "未命名";
    docObj["currentLayer"] = m_document ? m_document->currentLayer() : "0";
    root["document"] = docObj;

    // 图层
    root["layers"] = serializeLayers();

    // 图元
    root["entities"] = serializeEntities();

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_error = "无法打开文件写入: " + filePath;
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ProjectIO::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = "无法打开文件: " + filePath;
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        m_error = "JSON解析错误: " + parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    // 清除现有内容
    if (m_scene) m_scene->clear();

    // 恢复文档属性
    if (root.contains("document")) {
        QJsonObject docObj = root["document"].toObject();
        if (m_document) {
            m_document->setName(docObj["name"].toString("未命名"));
            if (docObj.contains("currentLayer")) {
                m_document->setCurrentLayer(docObj["currentLayer"].toString("0"));
            }
        }
    }

    // 恢复图层
    deserializeLayers(root);

    // 恢复图元
    deserializeEntities(root);

    return true;
}

QJsonObject ProjectIO::serializeLayers()
{
    QJsonObject layersObj;
    if (!m_document) return layersObj;

    QList<LayerInfo> layers = m_document->getAllLayers();
    QJsonArray layerArray;
    for (const LayerInfo &layer : layers) {
        QJsonObject obj;
        obj["name"] = layer.name;
        obj["color"] = layer.color.name();
        obj["visible"] = layer.visible;
        obj["locked"] = layer.locked;
        obj["frozen"] = layer.frozen;
        obj["plot"] = layer.plot;
        obj["lineType"] = layer.lineType;
        obj["lineWidth"] = layer.lineWidth;
        layerArray.append(obj);
    }
    layersObj["list"] = layerArray;
    return layersObj;
}

void ProjectIO::deserializeLayers(const QJsonObject &root)
{
    if (!m_document || !root.contains("layers")) return;
    QJsonObject layersObj = root["layers"].toObject();
    QJsonArray layerArray = layersObj["list"].toArray();

    // 清除现有图层（保留0层）
    // 注意：Document的图层管理需要扩展，这里直接添加

    for (const QJsonValue &val : layerArray) {
        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString();
        QColor color(obj["color"].toString("#ffffff"));
        m_document->addLayer(name, color);
        LayerInfo *info = m_document->getLayer(name);
        if (info) {
            info->visible = obj["visible"].toBool(true);
            info->locked = obj["locked"].toBool(false);
            info->frozen = obj["frozen"].toBool(false);
            info->plot = obj["plot"].toBool(true);
            info->lineType = obj["lineType"].toString("Continuous");
            info->lineWidth = obj["lineWidth"].toDouble(0.25);
        }
    }
}

QJsonObject ProjectIO::serializeEntities()
{
    QJsonObject entitiesObj;
    QJsonArray entityArray;
    if (!m_scene) return entitiesObj;

    for (QGraphicsItem *item : m_scene->items()) {
        QJsonObject obj;
        if (auto line = dynamic_cast<LineItem*>(item)) {
            obj["type"] = "line";
            obj["x1"] = line->startPoint().x();
            obj["y1"] = line->startPoint().y();
            obj["x2"] = line->endPoint().x();
            obj["y2"] = line->endPoint().y();
            obj["layer"] = line->layer();
            obj["color"] = line->color().name();
            obj["colorByLayer"] = line->isColorByLayer();
            obj["lineWidth"] = line->lineWidth();
            entityArray.append(obj);
        } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
            obj["type"] = "circle";
            obj["cx"] = circle->centerPoint().x();
            obj["cy"] = circle->centerPoint().y();
            obj["r"] = circle->radius();
            obj["layer"] = circle->layer();
            obj["color"] = circle->color().name();
            obj["colorByLayer"] = circle->isColorByLayer();
            obj["lineWidth"] = circle->lineWidth();
            entityArray.append(obj);
        } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
            obj["type"] = "arc";
            obj["cx"] = arc->centerPoint().x();
            obj["cy"] = arc->centerPoint().y();
            obj["r"] = arc->radius();
            obj["startAngle"] = arc->startAngle();
            obj["spanAngle"] = arc->spanAngle();
            obj["layer"] = arc->layer();
            obj["color"] = arc->color().name();
            obj["colorByLayer"] = arc->isColorByLayer();
            obj["lineWidth"] = arc->lineWidth();
            entityArray.append(obj);
        } else if (auto poly = dynamic_cast<PolylineItem*>(item)) {
            obj["type"] = "polyline";
            QJsonArray pts;
            for (const QPointF &p : poly->points()) {
                QJsonObject pt;
                pt["x"] = p.x();
                pt["y"] = p.y();
                pts.append(pt);
            }
            obj["points"] = pts;
            obj["closed"] = poly->isClosed();
            obj["layer"] = poly->layer();
            obj["color"] = poly->color().name();
            obj["colorByLayer"] = poly->isColorByLayer();
            obj["lineWidth"] = poly->lineWidth();
            entityArray.append(obj);
        } else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
            obj["type"] = "rectangle";
            obj["x1"] = rect->rectangle().topLeft().x();
            obj["y1"] = rect->rectangle().topLeft().y();
            obj["x2"] = rect->rectangle().bottomRight().x();
            obj["y2"] = rect->rectangle().bottomRight().y();
            obj["layer"] = rect->layer();
            obj["color"] = rect->color().name();
            obj["colorByLayer"] = rect->isColorByLayer();
            obj["lineWidth"] = rect->lineWidth();
            entityArray.append(obj);
        } else if (auto text = dynamic_cast<TextItem*>(item)) {
            obj["type"] = "text";
            obj["x"] = text->position().x();
            obj["y"] = text->position().y();
            obj["content"] = text->text();
            obj["height"] = text->textHeight();
            obj["layer"] = text->layer();
            obj["color"] = text->color().name();
            obj["colorByLayer"] = text->isColorByLayer();
            entityArray.append(obj);
        } else if (auto dim = dynamic_cast<DimensionItem*>(item)) {
            obj["type"] = "dimension";
            obj["x1"] = dim->startPoint().x();
            obj["y1"] = dim->startPoint().y();
            obj["x2"] = dim->endPoint().x();
            obj["y2"] = dim->endPoint().y();
            obj["dx"] = dim->dimPosition().x();
            obj["dy"] = dim->dimPosition().y();
            obj["dimType"] = static_cast<int>(dim->dimType());
            obj["layer"] = dim->layer();
            obj["color"] = dim->color().name();
            obj["colorByLayer"] = dim->isColorByLayer();
            entityArray.append(obj);
        }
    }
    entitiesObj["list"] = entityArray;
    return entitiesObj;
}

void ProjectIO::deserializeEntities(const QJsonObject &root)
{
    if (!m_scene || !root.contains("entities")) return;
    QJsonObject entitiesObj = root["entities"].toObject();
    QJsonArray entityArray = entitiesObj["list"].toArray();

    for (const QJsonValue &val : entityArray) {
        QJsonObject obj = val.toObject();
        QString type = obj["type"].toString();
        QString layer = obj["layer"].toString("0");
        QColor color(obj["color"].toString("#ffffff"));
        bool colorByLayer = obj["colorByLayer"].toBool(true);
        qreal lineWidth = obj["lineWidth"].toDouble(0.25);

        CadItem *item = nullptr;

        if (type == "line") {
            item = new LineItem(
                QPointF(obj["x1"].toDouble(), obj["y1"].toDouble()),
                QPointF(obj["x2"].toDouble(), obj["y2"].toDouble()));
        } else if (type == "circle") {
            item = new CircleItem(
                QPointF(obj["cx"].toDouble(), obj["cy"].toDouble()),
                obj["r"].toDouble());
        } else if (type == "arc") {
            item = new ArcItem(
                QPointF(obj["cx"].toDouble(), obj["cy"].toDouble()),
                obj["r"].toDouble(),
                obj["startAngle"].toDouble(),
                obj["spanAngle"].toDouble());
        } else if (type == "polyline") {
            QPolygonF points;
            QJsonArray pts = obj["points"].toArray();
            for (const QJsonValue &pv : pts) {
                QJsonObject p = pv.toObject();
                points.append(QPointF(p["x"].toDouble(), p["y"].toDouble()));
            }
            item = new PolylineItem(points, obj["closed"].toBool(false));
        } else if (type == "rectangle") {
            item = new RectangleItem(QRectF(QPointF(obj["x1"].toDouble(), obj["y1"].toDouble()), QPointF(obj["x2"].toDouble(), obj["y2"].toDouble())).normalized());
        } else if (type == "text") {
            item = new TextItem(
                QPointF(obj["x"].toDouble(), obj["y"].toDouble()),
                obj["content"].toString(),
                obj["height"].toDouble(2.5));
        } else if (type == "dimension") {
            QPointF p1(obj["x1"].toDouble(), obj["y1"].toDouble());
            QPointF p2(obj["x2"].toDouble(), obj["y2"].toDouble());
            QPointF dimPos(obj["dx"].toDouble(), obj["dy"].toDouble());
            DimensionItem::DimType dtype = static_cast<DimensionItem::DimType>(obj["dimType"].toInt(0));
            item = new DimensionItem(dtype, p1, p2, dimPos);
        }

        if (item) {
            item->setLayer(layer);
            item->setColor(color);
            item->setColorByLayer(colorByLayer);
            item->setLineWidth(lineWidth);
            m_scene->addItem(item);
        }
    }
}
