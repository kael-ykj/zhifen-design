#include "dxf_importer.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QtMath>

namespace Zhifen {

DxfImporter::DxfImporter() {}
DxfImporter::~DxfImporter() {}

bool DxfImporter::isWallLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("wall") || lower.contains("墙") || lower.contains("wall-") ||
           lower.contains("wall_") || lower.contains("承重墙") || lower.contains("剪力墙");
}

bool DxfImporter::isDoorWindowLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("door") || lower.contains("window") || lower.contains("门") ||
           lower.contains("窗") || lower.contains("dw") || lower.contains("门窗");
}

bool DxfImporter::isPipeLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("pipe") || lower.contains("管线") || lower.contains("弱电") ||
           lower.contains("强电") || lower.contains("给排水") || lower.contains("消防") ||
           lower.contains("暖通") || lower.contains("duct") || lower.contains("elec");
}

bool DxfImporter::isDimensionLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("dim") || lower.contains("标注") || lower.contains("尺寸") ||
           lower.contains("axis") || lower.contains("轴网") || lower.contains("标高");
}

bool DxfImporter::isHatchLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("hatch") || lower.contains("填充") || lower.contains("solid");
}

bool DxfImporter::isFurnitureLayer(const QString &layerName) {
    QString lower = layerName.toLower();
    return lower.contains("furniture") || lower.contains("家具") || lower.contains("洁具") ||
           lower.contains("设备") || lower.contains("desk") || lower.contains("chair");
}

QString DxfImporter::classifyLayer(const QString &layerName) {
    if (isWallLayer(layerName)) return "墙体";
    if (isDoorWindowLayer(layerName)) return "门窗";
    if (isPipeLayer(layerName)) return "管线";
    if (isDimensionLayer(layerName)) return "标注";
    if (isHatchLayer(layerName)) return "填充";
    if (isFurnitureLayer(layerName)) return "家具";
    return "其他";
}

bool DxfImporter::shouldKeepLayer(const QString &layerName, SimplifyMode mode) {
    if (mode == Simplify_None) return true;
    QString category = classifyLayer(layerName);
    if (mode == Simplify_Basic) {
        // 基础精简：保留墙体/门窗/管线
        return category == "墙体" || category == "门窗" || category == "管线" || category == "其他";
    }
    if (mode == Simplify_Aggressive) {
        // 深度精简：仅保留墙体
        return category == "墙体";
    }
    return true;
}

void DxfImporter::simplify(DxfImportResult &result, SimplifyMode mode) {
    if (mode == Simplify_None) return;

    QList<DxfEntity> kept;
    QMap<QString, int> layerCount;

    for (const auto &entity : result.entities) {
        if (shouldKeepLayer(entity.layer, mode)) {
            kept.append(entity);
            layerCount[entity.layer]++;
        }
    }

    result.entities = kept;

    // 更新图层统计
    for (auto &layer : result.layers) {
        layer.entityCount = layerCount.value(layer.name, 0);
    }

    result.warnings.append(QString("精简模式: %1，保留%2个图元")
        .arg(mode == Simplify_Basic ? "基础" : "深度")
        .arg(kept.size()));
}

bool DxfImporter::parseDxf(const QString &filePath, DxfImportResult &result) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errors.append("无法打开文件: " + filePath);
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QMap<QString, LayerInfo> layerMap;
    QList<DxfEntity> entities;

    QString currentSection;
    DxfEntity currentEntity;
    bool inEntities = false;
    bool inLayerTable = false;

    QString code;
    QString value;
    int lineNum = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNum++;

        if (lineNum % 2 == 1) {
            code = line;
            continue;
        }
        value = line;

        // 段切换
        if (code == "0" && value == "SECTION") {
            continue;
        }
        if (code == "2" && currentSection.isEmpty()) {
            currentSection = value;
            if (currentSection == "ENTITIES") inEntities = true;
            if (currentSection == "TABLES") inLayerTable = false;
            continue;
        }
        if (code == "0" && value == "ENDSEC") {
            currentSection.clear();
            inEntities = false;
            continue;
        }

        // 图层表
        if (currentSection == "TABLES" && code == "2" && value == "LAYER") {
            inLayerTable = true;
            continue;
        }
        if (inLayerTable && code == "0" && value == "LAYER") {
            // 新图层定义开始
            continue;
        }
        if (inLayerTable && code == "2") {
            LayerInfo li;
            li.name = value;
            layerMap[value] = li;
            continue;
        }
        if (inLayerTable && code == "62") {
            // 颜色
            int colorIdx = value.toInt();
            QColor c = Qt::black;
            if (colorIdx == 1) c = Qt::red;
            else if (colorIdx == 2) c = Qt::yellow;
            else if (colorIdx == 3) c = Qt::green;
            else if (colorIdx == 4) c = Qt::cyan;
            else if (colorIdx == 5) c = Qt::blue;
            else if (colorIdx == 6) c = Qt::magenta;
            else if (colorIdx == 7) c = Qt::white;
            if (!layerMap.isEmpty()) {
                auto it = layerMap.end(); --it;
                it.value().color = c;
            }
            continue;
        }

        // 实体解析
        if (inEntities) {
            if (code == "0") {
                // 保存上一个实体
                if (currentEntity.type != Dxf_Unknown) {
                    entities.append(currentEntity);
                }
                // 开始新实体
                currentEntity = DxfEntity();
                if (value == "LINE") currentEntity.type = Dxf_Line;
                else if (value == "LWPOLYLINE") currentEntity.type = Dxf_LwPolyline;
                else if (value == "CIRCLE") currentEntity.type = Dxf_Circle;
                else if (value == "ARC") currentEntity.type = Dxf_Arc;
                else if (value == "TEXT" || value == "MTEXT") currentEntity.type = Dxf_Text;
                else currentEntity.type = Dxf_Unknown;
                continue;
            }
            if (code == "8") {
                currentEntity.layer = value;
                if (!layerMap.contains(value)) {
                    LayerInfo li;
                    li.name = value;
                    layerMap[value] = li;
                }
                continue;
            }
            if (code == "62") {
                int colorIdx = value.toInt();
                if (colorIdx == 1) currentEntity.color = Qt::red;
                else if (colorIdx == 2) currentEntity.color = Qt::yellow;
                else if (colorIdx == 3) currentEntity.color = Qt::green;
                else if (colorIdx == 4) currentEntity.color = Qt::cyan;
                else if (colorIdx == 5) currentEntity.color = Qt::blue;
                else if (colorIdx == 6) currentEntity.color = Qt::magenta;
                continue;
            }
            if (currentEntity.type == Dxf_Line) {
                if (code == "10") currentEntity.start.setX(value.toDouble());
                else if (code == "20") currentEntity.start.setY(value.toDouble());
                else if (code == "11") currentEntity.end.setX(value.toDouble());
                else if (code == "21") currentEntity.end.setY(value.toDouble());
            } else if (currentEntity.type == Dxf_Circle) {
                if (code == "10") currentEntity.center.setX(value.toDouble());
                else if (code == "20") currentEntity.center.setY(value.toDouble());
                else if (code == "40") currentEntity.radius = value.toDouble();
            } else if (currentEntity.type == Dxf_Arc) {
                if (code == "10") currentEntity.center.setX(value.toDouble());
                else if (code == "20") currentEntity.center.setY(value.toDouble());
                else if (code == "40") currentEntity.radius = value.toDouble();
                else if (code == "50") currentEntity.startAngle = value.toDouble();
                else if (code == "51") currentEntity.endAngle = value.toDouble();
            } else if (currentEntity.type == Dxf_Text) {
                if (code == "10") currentEntity.start.setX(value.toDouble());
                else if (code == "20") currentEntity.start.setY(value.toDouble());
                else if (code == "40") currentEntity.textHeight = value.toDouble();
                else if (code == "1") currentEntity.text = value;
            } else if (currentEntity.type == Dxf_LwPolyline) {
                if (code == "10") {
                    QPointF p;
                    p.setX(value.toDouble());
                    currentEntity.vertices.append(p);
                } else if (code == "20" && !currentEntity.vertices.isEmpty()) {
                    currentEntity.vertices.last().setY(value.toDouble());
                } else if (code == "70") {
                    currentEntity.closed = (value.toInt() & 1) != 0;
                }
            }
        }
    }

    // 保存最后一个实体
    if (currentEntity.type != Dxf_Unknown) {
        entities.append(currentEntity);
    }

    file.close();

    result.entities = entities;
    result.layers = layerMap.values();

    // 统计每个图层的图元数
    QMap<QString, int> countMap;
    for (const auto &e : entities) countMap[e.layer]++;
    for (auto &l : result.layers) l.entityCount = countMap.value(l.name, 0);

    result.warnings.append(QString("解析完成: %1个图元, %2个图层").arg(entities.size()).arg(result.layers.size()));

    return !entities.isEmpty();
}

DxfImportResult DxfImporter::importFromFile(const QString &filePath, SimplifyMode mode) {
    DxfImportResult result;

    if (!parseDxf(filePath, result)) {
        result.success = false;
        return result;
    }

    // 精简
    simplify(result, mode);

    m_layers = result.layers;
    for (const auto &l : m_layers) {
        m_layerVisible[l.name] = true;
        m_layerLocked[l.name] = false;
    }

    result.success = true;
    return result;
}

void DxfImporter::renderToScene(const DxfImportResult &result, QGraphicsScene *scene, bool lockBottom) {
    if (!scene) return;

    QPen defaultPen(QColor(80, 80, 80), 0.5);

    for (const auto &entity : result.entities) {
        if (!m_layerVisible.value(entity.layer, true)) continue;

        QPen pen = defaultPen;
        if (entity.color != Qt::black) pen.setColor(entity.color);

        bool locked = lockBottom || m_layerLocked.value(entity.layer, false);

        if (entity.type == Dxf_Line) {
            QGraphicsLineItem *line = scene->addLine(QLineF(entity.start, entity.end), pen);
            if (locked) {
                line->setFlag(QGraphicsItem::ItemIsSelectable, false);
                line->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        } else if (entity.type == Dxf_Circle) {
            QGraphicsEllipseItem *circle = scene->addEllipse(
                QRectF(entity.center.x() - entity.radius, entity.center.y() - entity.radius,
                       entity.radius * 2, entity.radius * 2), pen);
            if (locked) {
                circle->setFlag(QGraphicsItem::ItemIsSelectable, false);
                circle->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        } else if (entity.type == Dxf_Arc) {
            QPainterPath path;
            path.arcMoveTo(QRectF(entity.center.x() - entity.radius, entity.center.y() - entity.radius,
                                  entity.radius * 2, entity.radius * 2), entity.startAngle);
            path.arcTo(QRectF(entity.center.x() - entity.radius, entity.center.y() - entity.radius,
                              entity.radius * 2, entity.radius * 2), entity.startAngle,
                       entity.endAngle - entity.startAngle);
            QGraphicsPathItem *arc = scene->addPath(path, pen);
            if (locked) {
                arc->setFlag(QGraphicsItem::ItemIsSelectable, false);
                arc->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        } else if (entity.type == Dxf_Text) {
            QGraphicsSimpleTextItem *text = scene->addSimpleText(entity.text);
            text->setPos(entity.start);
            text->setBrush(QBrush(pen.color()));
            QFont f = text->font();
            f.setPointSizeF(entity.textHeight);
            text->setFont(f);
            if (locked) {
                text->setFlag(QGraphicsItem::ItemIsSelectable, false);
                text->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        } else if (entity.type == Dxf_LwPolyline && entity.vertices.size() >= 2) {
            QPainterPath path;
            path.moveTo(entity.vertices[0]);
            for (int i = 1; i < entity.vertices.size(); i++) {
                path.lineTo(entity.vertices[i]);
            }
            if (entity.closed) path.closeSubpath();
            QGraphicsPathItem *poly = scene->addPath(path, pen);
            if (locked) {
                poly->setFlag(QGraphicsItem::ItemIsSelectable, false);
                poly->setFlag(QGraphicsItem::ItemIsMovable, false);
            }
        }
    }
}

void DxfImporter::setLayerVisible(const QString &layerName, bool visible) {
    m_layerVisible[layerName] = visible;
}

void DxfImporter::setLayerLocked(const QString &layerName, bool locked) {
    m_layerLocked[layerName] = locked;
}

} // namespace Zhifen
