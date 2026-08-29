#include "version_manager.h"
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsItemGroup>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

VersionManager::VersionManager() {}
VersionManager::~VersionManager() { clear(); }

VersionManager& VersionManager::instance() {
    static VersionManager inst;
    return inst;
}

void VersionManager::analyzeScene(QGraphicsScene *scene, int &entityCount, int &deviceCount, qreal &feederLength) {
    entityCount = 0;
    deviceCount = 0;
    feederLength = 0;
    if (!scene) return;

    for (QGraphicsItem *item : scene->items()) {
        entityCount++;
        QString type = item->data(0).toString();
        if (type == "DEVICE" || type == "BLOCK_REF") {
            deviceCount++;
        }
        if (item->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(item);
            feederLength += line->line().length();
        }
    }
}

QString VersionManager::serializeScene(QGraphicsScene *scene) {
    if (!scene) return QString();

    QJsonArray itemsArray;
    for (QGraphicsItem *item : scene->items()) {
        QJsonObject obj;
        obj["type"] = item->type();
        obj["x"] = item->pos().x();
        obj["y"] = item->pos().y();
        obj["rotation"] = item->rotation();
        obj["scale"] = item->scale();
        obj["zValue"] = item->zValue();
        obj["visible"] = item->isVisible();
        obj["data0"] = item->data(0).toString();
        obj["data1"] = item->data(1).toString();

        if (item->type() == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = static_cast<QGraphicsLineItem*>(item);
            obj["line_x1"] = line->line().x1();
            obj["line_y1"] = line->line().y1();
            obj["line_x2"] = line->line().x2();
            obj["line_y2"] = line->line().y2();
        } else if (item->type() == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = static_cast<QGraphicsRectItem*>(item);
            obj["rect_x"] = rect->rect().x();
            obj["rect_y"] = rect->rect().y();
            obj["rect_w"] = rect->rect().width();
            obj["rect_h"] = rect->rect().height();
        } else if (item->type() == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ell = static_cast<QGraphicsEllipseItem*>(item);
            obj["ell_x"] = ell->rect().x();
            obj["ell_y"] = ell->rect().y();
            obj["ell_w"] = ell->rect().width();
            obj["ell_h"] = ell->rect().height();
        } else if (item->type() == QGraphicsTextItem::Type) {
            QGraphicsTextItem *text = static_cast<QGraphicsTextItem*>(item);
            obj["text"] = text->toPlainText();
        }
        itemsArray.append(obj);
    }

    QJsonObject root;
    root["items"] = itemsArray;
    root["itemCount"] = itemsArray.size();
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

bool VersionManager::deserializeScene(QGraphicsScene *scene, const QString &data) {
    if (!scene || data.isEmpty()) return false;

    scene->clear();
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray itemsArray = root["items"].toArray();

    for (const QJsonValue &val : itemsArray) {
        QJsonObject obj = val.toObject();
        int type = obj["type"].toInt();
        QGraphicsItem *item = nullptr;

        if (type == QGraphicsLineItem::Type) {
            QGraphicsLineItem *line = new QGraphicsLineItem(
                obj["line_x1"].toDouble(), obj["line_y1"].toDouble(),
                obj["line_x2"].toDouble(), obj["line_y2"].toDouble());
            item = line;
        } else if (type == QGraphicsRectItem::Type) {
            QGraphicsRectItem *rect = new QGraphicsRectItem(
                obj["rect_x"].toDouble(), obj["rect_y"].toDouble(),
                obj["rect_w"].toDouble(), obj["rect_h"].toDouble());
            item = rect;
        } else if (type == QGraphicsEllipseItem::Type) {
            QGraphicsEllipseItem *ell = new QGraphicsEllipseItem(
                obj["ell_x"].toDouble(), obj["ell_y"].toDouble(),
                obj["ell_w"].toDouble(), obj["ell_h"].toDouble());
            item = ell;
        } else if (type == QGraphicsTextItem::Type) {
            QGraphicsTextItem *text = new QGraphicsTextItem(obj["text"].toString());
            item = text;
        }

        if (item) {
            item->setPos(obj["x"].toDouble(), obj["y"].toDouble());
            item->setRotation(obj["rotation"].toDouble());
            item->setScale(obj["scale"].toDouble());
            item->setZValue(obj["zValue"].toDouble());
            item->setVisible(obj["visible"].toBool());
            item->setData(0, obj["data0"].toString());
            item->setData(1, obj["data1"].toString());
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
            scene->addItem(item);
        }
    }
    return true;
}

int VersionManager::saveVersion(QGraphicsScene *scene, const QString &name, const QString &remark) {
    if (!scene) return -1;

    // 标记旧版本非当前
    for (VersionSnapshot *v : m_versions) {
        v->isCurrent = false;
    }

    VersionSnapshot *snap = new VersionSnapshot();
    snap->versionNo = m_nextVersionNo++;
    snap->name = name;
    snap->remark = remark;
    snap->created = QDateTime::currentDateTime();
    snap->operatorName = "设计";
    snap->sceneData = serializeScene(scene);
    snap->isCurrent = true;
    analyzeScene(scene, snap->entityCount, snap->deviceCount, snap->feederLength);

    m_versions[snap->versionNo] = snap;
    m_currentVersion = snap->versionNo;
    return snap->versionNo;
}

bool VersionManager::rollbackToVersion(int versionNo, QGraphicsScene *scene) {
    if (!m_versions.contains(versionNo) || !scene) return false;

    VersionSnapshot *target = m_versions[versionNo];
    return deserializeScene(scene, target->sceneData);
}

bool VersionManager::deleteVersion(int versionNo) {
    if (!m_versions.contains(versionNo)) return false;
    if (m_versions[versionNo]->isCurrent) return false; // 不能删除当前版本
    delete m_versions[versionNo];
    m_versions.remove(versionNo);
    return true;
}

VersionSnapshot* VersionManager::version(int versionNo) {
    return m_versions.value(versionNo, nullptr);
}

QList<VersionSnapshot*> VersionManager::allVersions() const {
    return m_versions.values();
}

void VersionManager::clear() {
    for (VersionSnapshot *v : m_versions) {
        delete v;
    }
    m_versions.clear();
    m_currentVersion = 0;
    m_nextVersionNo = 1;
}

QString VersionManager::compareVersions(int v1, int v2) const {
    VersionSnapshot *snap1 = m_versions.value(v1);
    VersionSnapshot *snap2 = m_versions.value(v2);
    if (!snap1 || !snap2) return QString();

    QString result;
    result += QString("版本对比: V%1 vs V%2\n").arg(v1).arg(v2);
    result += QString("图元数量: %1 -> %2 (变化: %3)\n")
        .arg(snap1->entityCount).arg(snap2->entityCount)
        .arg(snap2->entityCount - snap1->entityCount);
    result += QString("器件数量: %1 -> %2 (变化: %3)\n")
        .arg(snap1->deviceCount).arg(snap2->deviceCount)
        .arg(snap2->deviceCount - snap1->deviceCount);
    result += QString("馈线长度: %1m -> %2m (变化: %3m)\n")
        .arg(snap1->feederLength).arg(snap2->feederLength)
        .arg(snap2->feederLength - snap1->feederLength);
    result += QString("V%1时间: %2\n").arg(v1).arg(snap1->created.toString("yyyy-MM-dd hh:mm:ss"));
    result += QString("V%2时间: %3\n").arg(v2).arg(snap2->created.toString("yyyy-MM-dd hh:mm:ss"));
    return result;
}

bool VersionManager::saveToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "[Versions]\n";
    out << "Count=" << m_versions.size() << "\n";
    out << "Current=" << m_currentVersion << "\n";
    out << "NextNo=" << m_nextVersionNo << "\n";
    for (VersionSnapshot *v : m_versions) {
        out << QString("\n[Version:%1]\n").arg(v->versionNo);
        out << "Name=" << v->name << "\n";
        out << "Remark=" << v->remark << "\n";
        out << "Created=" << v->created.toString(Qt::ISODate) << "\n";
        out << "Operator=" << v->operatorName << "\n";
        out << "EntityCount=" << v->entityCount << "\n";
        out << "DeviceCount=" << v->deviceCount << "\n";
        out << "FeederLength=" << v->feederLength << "\n";
        out << "IsCurrent=" << (v->isCurrent ? "1" : "0") << "\n";
    }
    file.close();
    return true;
}

bool VersionManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    clear();
    VersionSnapshot *current = nullptr;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("[Version:")) {
            int no = line.mid(9, line.length() - 10).toInt();
            current = new VersionSnapshot();
            current->versionNo = no;
            m_versions[no] = current;
            if (no >= m_nextVersionNo) m_nextVersionNo = no + 1;
        } else if (current && line.startsWith("Name=")) {
            current->name = line.mid(5);
        } else if (current && line.startsWith("Remark=")) {
            current->remark = line.mid(7);
        } else if (current && line.startsWith("Created=")) {
            current->created = QDateTime::fromString(line.mid(8), Qt::ISODate);
        } else if (current && line.startsWith("Operator=")) {
            current->operatorName = line.mid(9);
        } else if (current && line.startsWith("EntityCount=")) {
            current->entityCount = line.mid(12).toInt();
        } else if (current && line.startsWith("DeviceCount=")) {
            current->deviceCount = line.mid(12).toInt();
        } else if (current && line.startsWith("FeederLength=")) {
            current->feederLength = line.mid(13).toDouble();
        } else if (current && line.startsWith("IsCurrent=")) {
            current->isCurrent = line.mid(10) == "1";
            if (current->isCurrent) m_currentVersion = current->versionNo;
        }
    }
    file.close();
    return !m_versions.isEmpty();
}

} // namespace Zhifen
