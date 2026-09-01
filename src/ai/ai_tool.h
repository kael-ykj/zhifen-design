#ifndef AI_TOOL_H
#define AI_TOOL_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QGraphicsScene>

namespace Zhifen {

// ==================== 建筑底图AI精简 ====================
struct SimplifyResult {
    bool success = false;
    int totalLayers = 0;        // 总图层数
    int keptLayers = 0;         // 保留图层数
    int removedLayers = 0;      // 删除图层数
    int totalEntities = 0;      // 总图元数
    int keptEntities = 0;       // 保留图元数
    int removedEntities = 0;    // 删除图元数
    QStringList keptLayerNames; // 保留的图层名
    QStringList removedLayerNames; // 删除的图层名
    QString report;             // 精简报告
};

class AISimplifyTool {
public:
    // 分析图层并自动精简
    static SimplifyResult analyzeAndSimplify(QGraphicsScene *scene, 
                                               bool keepWalls = true,
                                               bool keepDoors = true,
                                               bool keepPipes = true,
                                               bool keepDimensions = false);
    
    // 判断图层类型
    static QString classifyLayer(const QString &layerName);
    
    // 判断是否为墙体图层
    static bool isWallLayer(const QString &layerName);
    
    // 判断是否为门窗图层
    static bool isDoorWindowLayer(const QString &layerName);
    
    // 判断是否为管线图层
    static bool isPipeLayer(const QString &layerName);
    
    // 判断是否为标注图层
    static bool isDimensionLayer(const QString &layerName);
    
    // 判断是否为家具图层
    static bool isFurnitureLayer(const QString &layerName);
    
    // 判断是否为填充图层
    static bool isHatchLayer(const QString &layerName);
};

// ==================== 自动布放建议 ====================
struct AntennaSuggestion {
    QPointF position;            // 推荐位置
    qreal coverageRadius = 0;    // 覆盖半径(米)
    qreal estimatedPower = 0;    // 估算输入功率(dBm)
    QString antennaType;         // 推荐天线类型
    QString reason;              // 推荐理由
};

struct AutoPlaceResult {
    bool success = false;
    qreal totalArea = 0;         // 总面积(平方米)
    int suggestedAntennaCount = 0; // 建议天线数量
    QList<AntennaSuggestion> suggestions; // 天线建议列表
    qreal avgSpacing = 0;        // 平均间距(米)
    qreal estimatedCoverageRate = 0; // 估算覆盖率(%)
    QString report;              // 布放建议报告
};

class AutoPlaceTool {
public:
    // 根据场景范围自动计算天线布放建议
    static AutoPlaceResult calculate(QGraphicsScene *scene,
                                      qreal targetPower = -85.0,
                                      qreal antennaGain = 2.0,
                                      qreal txPower = 15.0,
                                      qreal coverageRadius = 15.0,
                                      const QString &band = "4G");
    
    // 计算单天线覆盖面积
    static qreal calculateCoverageArea(qreal radius);
    
    // 计算需要的天线数量
    static int calculateAntennaCount(qreal area, qreal coverageRadius, qreal overlap = 0.2);
    
    // 生成网格布放位置
    static QList<QPointF> generateGridPositions(const QRectF &area, qreal spacing);
};

// ==================== 设备材料估算 ====================
struct MaterialEstimateItem {
    QString category;            // 类别
    QString name;                // 名称
    QString model;               // 型号
    qreal quantity = 0;          // 数量
    QString unit;                // 单位
    qreal unitPrice = 0;         // 单价(元)
    qreal totalPrice = 0;        // 总价(元)
    QString remark;              // 备注
};

struct MaterialEstimateResult {
    bool success = false;
    qreal totalArea = 0;         // 总面积(平方米)
    int floorCount = 1;          // 楼层数
    QList<MaterialEstimateItem> mainMaterials; // 主材
    QList<MaterialEstimateItem> auxMaterials;  // 辅材
    qreal mainMaterialCost = 0;  // 主材费用
    qreal auxMaterialCost = 0;   // 辅材费用
    qreal totalCost = 0;         // 总费用
    qreal costPerSquareMeter = 0; // 每平米造价
    QString report;              // 估算报告
};

class MaterialEstimateTool {
public:
    // 根据面积和楼层数估算设备材料
    static MaterialEstimateResult estimate(qreal area, 
                                             int floorCount = 1,
                                             const QString &sceneType = "商业综合体",
                                             const QString &band = "4G",
                                             bool include5G = false);
    
    // 根据场景类型获取单位面积指标
    static qreal getCostPerSquareMeter(const QString &sceneType, const QString &band);
    
    // 根据面积计算天线数量
    static int estimateAntennaCount(qreal area, const QString &sceneType);
    
    // 根据天线数量计算馈线长度
    static qreal estimateFeederLength(int antennaCount, qreal area);
    
    // 生成主材清单
    static QList<MaterialEstimateItem> generateMainMaterials(int antennaCount, 
                                                                qreal feederLength,
                                                                const QString &band,
                                                                bool include5G);
    
    // 生成辅材清单
    static QList<MaterialEstimateItem> generateAuxMaterials(int antennaCount,
                                                               qreal feederLength);
};

} // namespace Zhifen

#endif // AI_TOOL_H
