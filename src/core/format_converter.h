#ifndef FORMAT_CONVERTER_H
#define FORMAT_CONVERTER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QGraphicsScene>

namespace Zhifen {

// 支持的格式类型
enum FormatType {
    Format_ZhiFen = 0,    // 智分Design
    Format_TianYue = 1,   // 天越
    Format_AIDP = 2,      // AIDP
    Format_DiFu = 3,      // 迪弗
    Format_Unknown = 99
};

// 外部图元数据结构（通用中间格式）
struct ExternalEntity {
    QString id;            // 图元ID
    QString type;          // 图元类型（source/antenna/device/feeder/text）
    QString model;         // 型号
    QString name;          // 名称
    QPointF position;      // 位置
    QPointF startPos;      // 起点（馈线）
    QPointF endPos;        // 终点（馈线）
    qreal rotation = 0;    // 旋转角度
    qreal scale = 1.0;     // 缩放
    QString layer;         // 图层
    QMap<QString, QString> properties; // 自定义属性
    QString sourceFormat;  // 来源格式
};

// 图元映射关系
struct EntityMapping {
    QString sourceModel;   // 源型号
    QString targetModel;   // 目标型号
    QString sourceType;    // 源类型
    QString targetType;    // 目标类型
    qreal scaleFactor = 1.0; // 缩放因子
    QString note;          // 备注
};

// 转换结果项
struct ConversionResultItem {
    QString entityId;
    QString entityType;
    QString entityModel;
    bool success = false;
    QString message;
    QString warning;
};

// 转换报告
struct ConversionReport {
    FormatType sourceFormat;
    FormatType targetFormat;
    QDateTime startTime;
    QDateTime endTime;
    int totalEntities = 0;
    int successCount = 0;
    int failedCount = 0;
    int warningCount = 0;
    QList<ConversionResultItem> items;
    QString summary;

    QString toString() const;
};

// 格式转换器
class FormatConverter
{
public:
    static FormatConverter& instance();
    ~FormatConverter();

    // 格式识别
    FormatType detectFormat(const QString &filePath);
    QString formatName(FormatType type) const;

    // 图元映射表
    bool loadMappingTable(const QString &filePath);
    bool saveMappingTable(const QString &filePath);
    void addMapping(const EntityMapping &mapping);
    EntityMapping findMapping(const QString &sourceModel, FormatType sourceFormat) const;
    QList<EntityMapping> allMappings() const { return m_mappings; }
    void initDefaultMappings();

    // 导入转换
    ConversionReport importFromFormat(const QString &filePath, FormatType format,
                                        QGraphicsScene *scene);
    ConversionReport importTianYue(const QString &filePath, QGraphicsScene *scene);
    ConversionReport importAIDP(const QString &filePath, QGraphicsScene *scene);
    ConversionReport importDiFu(const QString &filePath, QGraphicsScene *scene);

    // 导出转换
    ConversionReport exportToFormat(QGraphicsScene *scene, const QString &filePath,
                                      FormatType format);
    ConversionReport exportTianYue(QGraphicsScene *scene, const QString &filePath);
    ConversionReport exportAIDP(QGraphicsScene *scene, const QString &filePath);

    // 通用解析（基于DXF的简化解析）
    QList<ExternalEntity> parseDXF(const QString &filePath);
    bool writeDXF(const QList<ExternalEntity> &entities, const QString &filePath);

    // 场景与外部图元互转
    QList<ExternalEntity> sceneToEntities(QGraphicsScene *scene);
    bool entitiesToScene(const QList<ExternalEntity> &entities, QGraphicsScene *scene,
                          ConversionReport &report);

private:
    FormatConverter();
    QList<EntityMapping> m_mappings;

    // 创建图元
    bool createEntityInScene(const ExternalEntity &entity, QGraphicsScene *scene,
                              ConversionResultItem &result);
};

} // namespace Zhifen

#endif // FORMAT_CONVERTER_H
