#ifndef BATCH_IMPORTER_H
#define BATCH_IMPORTER_H

#include <QString>
#include <QList>
#include <QPointF>
#include <QGraphicsScene>

namespace Zhifen {

// 导入器件记录
struct ImportDeviceRecord {
    QString type;        // 器件类型：全向吸顶天线/壁挂天线/二功分器/三功分器/四功分器/5dB耦合器等
    qreal x = 0;         // X坐标
    qreal y = 0;         // Y坐标
    QString model;       // 型号
    QString deviceId;    // 编号
    qreal rotation = 0;  // 旋转角度
    qreal txPower = 15;  // 发射功率(dBm)
    bool valid = false;
};

// 导入结果
struct BatchImportResult {
    bool success = false;
    QList<ImportDeviceRecord> records;
    int importedCount = 0;
    int failedCount = 0;
    QStringList errors;
    QStringList warnings;
};

// 批量导入器
class BatchImporter
{
public:
    BatchImporter();
    ~BatchImporter();

    // 从CSV文件导入
    BatchImportResult importFromCsv(const QString &filePath);

    // 将导入记录放置到场景
    int placeToScene(const BatchImportResult &result, QGraphicsScene *scene, QPointF offset = QPointF(0, 0));

    // 获取导入模板说明
    static QString templateDescription();

    // 获取导入模板CSV内容
    static QString templateCsv();

    // 器件类型映射
    static QString normalizeDeviceType(const QString &type);

private:
    // 解析一行CSV
    ImportDeviceRecord parseLine(const QString &line, int lineNum);

    // 校验记录
    bool validateRecord(const ImportDeviceRecord &record, QString &error);
};

} // namespace Zhifen

#endif // BATCH_IMPORTER_H
