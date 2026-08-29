#include "format_converter.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsItemGroup>
#include <QPen>
#include <QBrush>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

QString ConversionReport::toString() const {
    QString result;
    result += "=== 格式转换报告 ===\n";
    result += QString("源格式: %1\n").arg(FormatConverter::instance().formatName(sourceFormat));
    result += QString("目标格式: %1\n").arg(FormatConverter::instance().formatName(targetFormat));
    result += QString("开始时间: %1\n").arg(startTime.toString("yyyy-MM-dd hh:mm:ss"));
    result += QString("结束时间: %1\n").arg(endTime.toString("yyyy-MM-dd hh:mm:ss"));
    result += QString("总图元数: %1\n").arg(totalEntities);
    result += QString("成功: %1  失败: %2  警告: %3\n").arg(successCount).arg(failedCount).arg(warningCount);
    result += QString("成功率: %1%\n").arg(totalEntities > 0 ? successCount * 100.0 / totalEntities : 0, 0, 'f', 1);
    if (!summary.isEmpty()) {
        result += QString("摘要: %1\n").arg(summary);
    }
    result += "\n=== 详细日志 ===\n";
    for (const ConversionResultItem &item : items) {
        QString status = item.success ? "成功" : "失败";
        result += QString("[%1] %2 (%3): %4\n")
            .arg(status).arg(item.entityType).arg(item.entityModel).arg(item.message);
        if (!item.warning.isEmpty()) {
            result += QString("  警告: %1\n").arg(item.warning);
        }
    }
    return result;
}

FormatConverter::FormatConverter() {
    initDefaultMappings();
}

FormatConverter::~FormatConverter() {}

FormatConverter& FormatConverter::instance() {
    static FormatConverter inst;
    return inst;
}

QString FormatConverter::formatName(FormatType type) const {
    switch (type) {
        case Format_ZhiFen: return "智分Design";
        case Format_TianYue: return "天越";
        case Format_AIDP: return "AIDP";
        case Format_DiFu: return "迪弗";
        default: return "未知格式";
    }
}

FormatType FormatConverter::detectFormat(const QString &filePath) {
    QFileInfo fi(filePath);
    QString ext = fi.suffix().toLower();

    // 基于扩展名识别
    if (ext == "zfd" || ext == "zhifen") return Format_ZhiFen;
    if (ext == "tyd" || ext == "tianyue") return Format_TianYue;
    if (ext == "aidp") return Format_AIDP;
    if (ext == "dfd" || ext == "difu") return Format_DiFu;

    // 基于文件内容识别（简化）
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString header = file.read(100);
        file.close();
        if (header.contains("TIANYUE", Qt::CaseInsensitive)) return Format_TianYue;
        if (header.contains("AIDP", Qt::CaseInsensitive)) return Format_AIDP;
        if (header.contains("DIFU", Qt::CaseInsensitive)) return Format_DiFu;
        if (header.contains("ZHIFEN", Qt::CaseInsensitive)) return Format_ZhiFen;
    }

    // DXF格式默认按天越处理（天越基于DXF）
    if (ext == "dxf") return Format_TianYue;

    return Format_Unknown;
}

void FormatConverter::initDefaultMappings() {
    // 天越 -> 智分Design 映射
    QList<QPair<QString, QString>> tianyueMappings = {
        {"OMB-450", "信源-450M"},
        {"OMB-900", "信源-900M"},
        {"OMB-1800", "信源-1800M"},
        {"OMB-2100", "信源-2100M"},
        {"OMB-2600", "信源-2600M"},
        {"OMB-3500", "信源-3500M"},
        {"ANT-OMNI", "全向吸顶天线"},
        {"ANT-PANEL", "板状天线"},
        {"ANT-WALL", "壁挂天线"},
        {"ANT-FIBER", "光纤天线"},
        {"ANT-LEAKY", "漏缆"},
        {"COU-5", "耦合器-5dB"},
        {"COU-6", "耦合器-6dB"},
        {"COU-7", "耦合器-7dB"},
        {"COU-10", "耦合器-10dB"},
        {"COU-12", "耦合器-12dB"},
        {"COU-15", "耦合器-15dB"},
        {"COU-20", "耦合器-20dB"},
        {"COU-30", "耦合器-30dB"},
        {"COU-40", "耦合器-40dB"},
        {"POW-2", "功分器-二功分"},
        {"POW-3", "功分器-三功分"},
        {"POW-4", "功分器-四功分"},
        {"COM-2", "合路器-二合路"},
        {"COM-3", "合路器-三合路"},
        {"COM-4", "合路器-四合路"},
        {"LOAD-50", "负载-50Ω"},
        {"ATT-5", "衰减器-5dB"},
        {"ATT-10", "衰减器-10dB"},
        {"FEED-1/2", "馈线-1/2\""},
        {"FEED-7/8", "馈线-7/8\""},
        {"FEED-5/4", "馈线-5/4\""},
        {"FEED-1-5/8", "馈线-1-5/8\""},
    };

    for (const auto &m : tianyueMappings) {
        EntityMapping mapping;
        mapping.sourceModel = m.first;
        mapping.targetModel = m.second;
        mapping.sourceType = "device";
        mapping.targetType = "device";
        mapping.note = "天越默认映射";
        m_mappings.append(mapping);
    }

    // AIDP -> 智分Design 映射（简化）
    QList<QPair<QString, QString>> aidpMappings = {
        {"BTS-900", "信源-900M"},
        {"BTS-1800", "信源-1800M"},
        {"BTS-2100", "信源-2100M"},
        {"BTS-3500", "信源-3500M"},
        {"ANT-CEIL", "全向吸顶天线"},
        {"ANT-DIR", "板状天线"},
        {"CPL-06", "耦合器-6dB"},
        {"CPL-10", "耦合器-10dB"},
        {"CPL-15", "耦合器-15dB"},
        {"SPL-2W", "功分器-二功分"},
        {"SPL-3W", "功分器-三功分"},
        {"CBM-2IN", "合路器-二合路"},
        {"CAB-12", "馈线-1/2\""},
        {"CAB-78", "馈线-7/8\""},
    };

    for (const auto &m : aidpMappings) {
        EntityMapping mapping;
        mapping.sourceModel = m.first;
        mapping.targetModel = m.second;
        mapping.sourceType = "device";
        mapping.targetType = "device";
        mapping.note = "AIDP默认映射";
        m_mappings.append(mapping);
    }
}

bool FormatConverter::loadMappingTable(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    m_mappings.clear();
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        QStringList parts = line.split(",");
        if (parts.size() >= 2) {
            EntityMapping mapping;
            mapping.sourceModel = parts[0].trimmed();
            mapping.targetModel = parts[1].trimmed();
            if (parts.size() >= 3) mapping.sourceType = parts[2].trimmed();
            if (parts.size() >= 4) mapping.targetType = parts[3].trimmed();
            if (parts.size() >= 5) mapping.note = parts[4].trimmed();
            m_mappings.append(mapping);
        }
    }
    file.close();
    return true;
}

bool FormatConverter::saveMappingTable(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "# 源型号,目标型号,源类型,目标类型,备注\n";
    for (const EntityMapping &m : m_mappings) {
        out << QString("%1,%2,%3,%4,%5\n")
            .arg(m.sourceModel).arg(m.targetModel)
            .arg(m.sourceType).arg(m.targetType).arg(m.note);
    }
    file.close();
    return true;
}

void FormatConverter::addMapping(const EntityMapping &mapping) {
    m_mappings.append(mapping);
}

EntityMapping FormatConverter::findMapping(const QString &sourceModel, FormatType sourceFormat) const {
    Q_UNUSED(sourceFormat);
    for (const EntityMapping &m : m_mappings) {
        if (m.sourceModel.compare(sourceModel, Qt::CaseInsensitive) == 0) {
            return m;
        }
    }
    // 模糊匹配
    for (const EntityMapping &m : m_mappings) {
        if (sourceModel.contains(m.sourceModel, Qt::CaseInsensitive) ||
            m.sourceModel.contains(sourceModel, Qt::CaseInsensitive)) {
            return m;
        }
    }
    return EntityMapping();
}

QList<ExternalEntity> FormatConverter::parseDXF(const QString &filePath) {
    QList<ExternalEntity> entities;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return entities;

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 简化的DXF解析：提取LINE/CIRCLE/TEXT/INSERT等图元
    QStringList lines = content.split("\n");
    ExternalEntity current;
    bool inEntity = false;

    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i].trimmed();
        if (line == "LINE" || line == "CIRCLE" || line == "TEXT" || line == "INSERT") {
            if (inEntity && !current.type.isEmpty()) {
                entities.append(current);
            }
            current = ExternalEntity();
            current.type = line.toLower();
            current.id = QString("ENT_%1").arg(entities.size());
            inEntity = true;
        } else if (inEntity && line == "ENDSEC") {
            if (!current.type.isEmpty()) {
                entities.append(current);
            }
            inEntity = false;
        } else if (inEntity && i + 1 < lines.size()) {
            bool ok;
            int code = line.toInt(&ok);
            if (ok && i + 1 < lines.size()) {
                QString value = lines[i + 1].trimmed();
                if (code == 10) current.position.setX(value.toDouble());
                if (code == 20) current.position.setY(value.toDouble());
                if (code == 11) current.endPos.setX(value.toDouble());
                if (code == 21) current.endPos.setY(value.toDouble());
                if (code == 2) current.model = value;
                if (code == 1) current.name = value;
                if (code == 8) current.layer = value;
                i++;
            }
        }
    }

    return entities;
}

bool FormatConverter::writeDXF(const QList<ExternalEntity> &entities, const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "0\nSECTION\n2\nENTITIES\n";

    for (const ExternalEntity &e : entities) {
        if (e.type == "line" || e.type == "feeder") {
            out << "0\nLINE\n";
            out << "8\n" << e.layer << "\n";
            out << "10\n" << e.startPos.x() << "\n";
            out << "20\n" << e.startPos.y() << "\n";
            out << "11\n" << e.endPos.x() << "\n";
            out << "21\n" << e.endPos.y() << "\n";
        } else if (e.type == "text") {
            out << "0\nTEXT\n";
            out << "8\n" << e.layer << "\n";
            out << "10\n" << e.position.x() << "\n";
            out << "20\n" << e.position.y() << "\n";
            out << "1\n" << e.name << "\n";
        } else if (e.type == "device" || e.type == "antenna" || e.type == "source") {
            out << "0\nINSERT\n";
            out << "8\n" << e.layer << "\n";
            out << "2\n" << e.model << "\n";
            out << "10\n" << e.position.x() << "\n";
            out << "20\n" << e.position.y() << "\n";
        }
    }

    out << "0\nENDSEC\n0\nEOF\n";
    file.close();
    return true;
}

QList<ExternalEntity> FormatConverter::sceneToEntities(QGraphicsScene *scene) {
    QList<ExternalEntity> entities;
    if (!scene) return entities;

    for (QGraphicsItem *item : scene->items()) {
        ExternalEntity e;
        e.id = QString("SCENE_%1").arg(entities.size());
        e.position = item->pos();
        e.rotation = item->rotation();
        e.scale = item->scale();

        QString type = item->data(0).toString();
        QString model = item->data(1).toString();

        if (type == "DEVICE" || type == "BLOCK_REF") {
            e.type = "device";
            e.model = model;
        } else if (type == "FEEDER") {
            e.type = "feeder";
            if (item->type() == QGraphicsLineItem::Type) {
                QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(item);
                e.startPos = line->line().p1();
                e.endPos = line->line().p2();
            }
        } else if (item->type() == QGraphicsLineItem::Type) {
            e.type = "line";
            QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(item);
            e.startPos = line->line().p1();
            e.endPos = line->line().p2();
        } else if (item->type() == QGraphicsTextItem::Type) {
            e.type = "text";
            QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(item);
            e.name = text->toPlainText();
        } else {
            e.type = "unknown";
        }

        e.sourceFormat = "ZhiFen";
        entities.append(e);
    }

    return entities;
}

bool FormatConverter::createEntityInScene(const ExternalEntity &entity, QGraphicsScene *scene,
                                            ConversionResultItem &result) {
    if (!scene) {
        result.success = false;
        result.message = "场景为空";
        return false;
    }

    result.entityId = entity.id;
    result.entityType = entity.type;
    result.entityModel = entity.model;

    if (entity.type == "feeder" || entity.type == "line") {
        QGraphicsLineItem *line = scene->addLine(entity.startPos.x(), entity.startPos.y(),
                                                    entity.endPos.x(), entity.endPos.y(),
                                                    QPen(QColor(0, 0, 0), 1));
        line->setData(0, "FEEDER");
        line->setData(1, entity.model);
        line->setFlag(QGraphicsItem::ItemIsSelectable, true);
        line->setFlag(QGraphicsItem::ItemIsMovable, true);
        result.success = true;
        result.message = QString("馈线已创建 (%1)").arg(entity.model);
        return true;
    } else if (entity.type == "device" || entity.type == "antenna" || entity.type == "source") {
        // 映射型号
        EntityMapping mapping = findMapping(entity.model, Format_TianYue);
        QString targetModel = mapping.targetModel.isEmpty() ? entity.model : mapping.targetModel;

        // 创建设备图元（简化为矩形+文字）
        QGraphicsItemGroup *group = new QGraphicsItemGroup();
        QGraphicsRectItem *rect = new QGraphicsRectItem(-5, -5, 10, 10, group);
        rect->setBrush(QBrush(QColor(200, 200, 255)));
        QGraphicsTextItem *text = new QGraphicsTextItem(targetModel, group);
        text->setPos(-20, 8);
        text->setFont(QFont("SimSun", 6));

        group->setPos(entity.position);
        group->setRotation(entity.rotation);
        group->setData(0, "DEVICE");
        group->setData(1, targetModel);
        group->setFlag(QGraphicsItem::ItemIsSelectable, true);
        group->setFlag(QGraphicsItem::ItemIsMovable, true);
        scene->addItem(group);

        result.success = true;
        result.message = QString("器件已创建: %1 -> %2").arg(entity.model).arg(targetModel);
        if (!mapping.targetModel.isEmpty() && mapping.targetModel != entity.model) {
            result.warning = QString("型号已映射: %1 -> %2").arg(entity.model).arg(targetModel);
        }
        return true;
    } else if (entity.type == "text") {
        QGraphicsTextItem *text = scene->addText(entity.name, QFont("SimSun", 8));
        text->setPos(entity.position);
        text->setData(0, "TEXT");
        text->setFlag(QGraphicsItem::ItemIsSelectable, true);
        result.success = true;
        result.message = QString("文字已创建: %1").arg(entity.name);
        return true;
    } else {
        result.success = false;
        result.message = QString("不支持的图元类型: %1").arg(entity.type);
        return false;
    }
}

bool FormatConverter::entitiesToScene(const QList<ExternalEntity> &entities, QGraphicsScene *scene,
                                        ConversionReport &report) {
    if (!scene) return false;

    for (const ExternalEntity &e : entities) {
        ConversionResultItem item;
        bool success = createEntityInScene(e, scene, item);
        report.items.append(item);
        report.totalEntities++;
        if (success) {
            report.successCount++;
        } else {
            report.failedCount++;
        }
        if (!item.warning.isEmpty()) {
            report.warningCount++;
        }
    }

    report.summary = QString("成功转换 %1/%2 个图元").arg(report.successCount).arg(report.totalEntities);
    return report.successCount > 0;
}

ConversionReport FormatConverter::importFromFormat(const QString &filePath, FormatType format,
                                                      QGraphicsScene *scene) {
    ConversionReport report;
    report.sourceFormat = format;
    report.targetFormat = Format_ZhiFen;
    report.startTime = QDateTime::currentDateTime();

    QList<ExternalEntity> entities;

    switch (format) {
        case Format_TianYue:
            entities = parseDXF(filePath);
            break;
        case Format_AIDP:
            entities = parseDXF(filePath); // AIDP也基于DXF
            break;
        case Format_DiFu:
            entities = parseDXF(filePath); // 迪弗也基于DXF
            break;
        default:
            report.summary = "不支持的格式";
            report.endTime = QDateTime::currentDateTime();
            return report;
    }

    entitiesToScene(entities, scene, report);
    report.endTime = QDateTime::currentDateTime();
    return report;
}

ConversionReport FormatConverter::importTianYue(const QString &filePath, QGraphicsScene *scene) {
    return importFromFormat(filePath, Format_TianYue, scene);
}

ConversionReport FormatConverter::importAIDP(const QString &filePath, QGraphicsScene *scene) {
    return importFromFormat(filePath, Format_AIDP, scene);
}

ConversionReport FormatConverter::importDiFu(const QString &filePath, QGraphicsScene *scene) {
    return importFromFormat(filePath, Format_DiFu, scene);
}

ConversionReport FormatConverter::exportToFormat(QGraphicsScene *scene, const QString &filePath,
                                                    FormatType format) {
    ConversionReport report;
    report.sourceFormat = Format_ZhiFen;
    report.targetFormat = format;
    report.startTime = QDateTime::currentDateTime();

    QList<ExternalEntity> entities = sceneToEntities(scene);
    bool success = writeDXF(entities, filePath);

    report.totalEntities = entities.size();
    report.successCount = success ? entities.size() : 0;
    report.failedCount = success ? 0 : entities.size();
    report.summary = success ? QString("成功导出 %1 个图元").arg(entities.size()) : "导出失败";
    report.endTime = QDateTime::currentDateTime();
    return report;
}

ConversionReport FormatConverter::exportTianYue(QGraphicsScene *scene, const QString &filePath) {
    return exportToFormat(scene, filePath, Format_TianYue);
}

ConversionReport FormatConverter::exportAIDP(QGraphicsScene *scene, const QString &filePath) {
    return exportToFormat(scene, filePath, Format_AIDP);
}

} // namespace Zhifen
