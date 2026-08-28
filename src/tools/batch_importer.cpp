#include "batch_importer.h"
#include "../devices/deviceitem.h"
#include "../entities/caditem.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QtMath>

namespace Zhifen {

BatchImporter::BatchImporter() {}
BatchImporter::~BatchImporter() {}

QString BatchImporter::templateDescription() {
    return "CSV导入格式说明:\n"
           "第一行为表头: 类型,X坐标,Y坐标,型号,编号,旋转角度,发射功率\n"
           "器件类型支持: 全向吸顶天线, 壁挂天线, 定向天线, 射灯天线, 二功分器, 三功分器, 四功分器, "
           "5dB耦合器, 6dB耦合器, 7dB耦合器, 10dB耦合器, 12dB耦合器, 15dB耦合器, 20dB耦合器, 30dB耦合器, 40dB耦合器, "
           "合路器, 电桥, RRU, BBU, 微基站, 直放站, 干放, pRRU, 扩展单元, POE交换机\n"
           "坐标单位: 毫米(mm)\n"
           "旋转角度: 度(0-360)\n"
           "发射功率: dBm(仅信源类器件有效)";
}

QString BatchImporter::templateCsv() {
    return "类型,X坐标,Y坐标,型号,编号,旋转角度,发射功率\n"
           "全向吸顶天线,1000,1000,ANT-OMNI-01,ANT-001,0,15\n"
           "二功分器,2000,1000,SPL-2,SPL-001,0,0\n"
           "5dB耦合器,3000,1000,CPL-5,CPL-001,0,0\n"
           "RRU,500,500,RRU-2100,SRC-001,0,43\n";
}

QString BatchImporter::normalizeDeviceType(const QString &type) {
    QString t = type.trimmed();
    // 常见别名映射
    if (t.contains("全向") || t.contains("吸顶") || t.toLower().contains("omni")) return "全向吸顶天线";
    if (t.contains("壁挂") || t.toLower().contains("wall")) return "壁挂天线";
    if (t.contains("定向") || t.toLower().contains("directional")) return "定向天线";
    if (t.contains("射灯") || t.contains("外引")) return "射灯天线";
    if (t.contains("栅格")) return "栅格天线";
    if (t.contains("二功分") || t.contains("2功分") || t == "功分器") return "二功分器";
    if (t.contains("三功分") || t.contains("3功分")) return "三功分器";
    if (t.contains("四功分") || t.contains("4功分")) return "四功分器";
    if (t.contains("5dB") || t.contains("5db")) return "5dB耦合器";
    if (t.contains("6dB") || t.contains("6db")) return "6dB耦合器";
    if (t.contains("7dB") || t.contains("7db")) return "7dB耦合器";
    if (t.contains("10dB") || t.contains("10db")) return "10dB耦合器";
    if (t.contains("12dB") || t.contains("12db")) return "12dB耦合器";
    if (t.contains("15dB") || t.contains("15db")) return "15dB耦合器";
    if (t.contains("20dB") || t.contains("20db")) return "20dB耦合器";
    if (t.contains("30dB") || t.contains("30db")) return "30dB耦合器";
    if (t.contains("40dB") || t.contains("40db")) return "40dB耦合器";
    if (t.contains("耦合")) return "10dB耦合器";
    if (t.contains("合路")) return "合路器";
    if (t.contains("电桥")) return "电桥";
    if (t.toUpper().contains("RRU")) return "RRU";
    if (t.toUpper().contains("BBU")) return "BBU";
    if (t.contains("微基站")) return "微基站";
    if (t.contains("直放站")) return "直放站";
    if (t.contains("干放")) return "干放";
    if (t.toLower().contains("prru")) return "pRRU";
    if (t.contains("扩展")) return "扩展单元";
    if (t.contains("POE") || t.contains("poe")) return "POE交换机";
    return t;
}

ImportDeviceRecord BatchImporter::parseLine(const QString &line, int lineNum) {
    ImportDeviceRecord record;
    QStringList parts = line.split(',');
    if (parts.size() < 3) {
        return record;
    }

    record.type = normalizeDeviceType(parts[0].trimmed());
    record.x = parts[1].trimmed().toDouble();
    record.y = parts[2].trimmed().toDouble();
    if (parts.size() > 3) record.model = parts[3].trimmed();
    if (parts.size() > 4) record.deviceId = parts[4].trimmed();
    if (parts.size() > 5) record.rotation = parts[5].trimmed().toDouble();
    if (parts.size() > 6) record.txPower = parts[6].trimmed().toDouble();

    record.valid = !record.type.isEmpty() && parts.size() >= 3;
    return record;
}

bool BatchImporter::validateRecord(const ImportDeviceRecord &record, QString &error) {
    if (record.type.isEmpty()) {
        error = "器件类型为空";
        return false;
    }
    if (qIsNaN(record.x) || qIsNaN(record.y)) {
        error = "坐标无效";
        return false;
    }
    return true;
}

BatchImportResult BatchImporter::importFromCsv(const QString &filePath) {
    BatchImportResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errors.append("无法打开文件: " + filePath);
        result.success = false;
        return result;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");

    int lineNum = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNum++;

        // 跳过表头和空行
        if (lineNum == 1 && (line.contains("类型") || line.contains("Type") || line.contains("type"))) {
            continue;
        }
        if (line.isEmpty()) continue;

        ImportDeviceRecord record = parseLine(line, lineNum);
        QString error;
        if (validateRecord(record, error)) {
            result.records.append(record);
            result.importedCount++;
        } else {
            result.failedCount++;
            result.errors.append(QString("第%1行: %2 - %3").arg(lineNum).arg(error).arg(line));
        }
    }

    file.close();

    result.success = result.importedCount > 0;
    if (result.failedCount > 0) {
        result.warnings.append(QString("%1条记录导入失败").arg(result.failedCount));
    }

    return result;
}

static DeviceType deviceTypeFromName(const QString &name) {
    if (name == "全向吸顶天线") return DevAntennaOmni;
    if (name == "壁挂天线" || name == "定向壁挂天线") return DevAntennaDirectional;
    if (name == "定向天线") return DevAntennaPanel;
    if (name == "射灯天线") return DevAntennaSpotlight;
    if (name == "外引天线") return DevAntennaExternal;
    if (name == "栅格天线") return DevAntennaGrid;
    if (name == "对数周期天线") return DevAntennaLPDA;
    if (name == "二功分器") return DevSplitter2;
    if (name == "三功分器") return DevSplitter3;
    if (name == "四功分器") return DevSplitter4;
    if (name == "5dB耦合器") return DevCoupler5;
    if (name == "6dB耦合器") return DevCoupler6;
    if (name == "7dB耦合器") return DevCoupler7;
    if (name == "10dB耦合器") return DevCoupler10;
    if (name == "12dB耦合器") return DevCoupler12;
    if (name == "15dB耦合器") return DevCoupler15;
    if (name == "20dB耦合器") return DevCoupler20;
    if (name == "30dB耦合器") return DevCoupler30;
    if (name == "40dB耦合器") return DevCoupler40;
    if (name == "合路器") return DevCombiner;
    if (name == "电桥") return DevHybrid;
    if (name == "RRU") return DevSourceRRU;
    if (name == "BBU") return DevSourceBBU;
    if (name == "微基站") return DevSourceMicro;
    if (name == "直放站") return DevSourceRepeater;
    if (name == "干放") return DevDryAmp;
    return DevUnknown;
}

int BatchImporter::placeToScene(const BatchImportResult &result, QGraphicsScene *scene, QPointF offset) {
    if (!scene) return 0;

    int placed = 0;
    for (const auto &record : result.records) {
        DeviceType dt = deviceTypeFromName(record.type);
        DeviceItem *device = new DeviceItem(dt);
        device->setPos(record.x + offset.x(), record.y + offset.y());
        if (!record.model.isEmpty()) device->setModel(record.model);
        if (record.rotation != 0) device->setRotation(record.rotation);
        scene->addItem(device);
        placed++;
    }

    return placed;
}

} // namespace Zhifen
