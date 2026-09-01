#include "ai/ai_tool.h"
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <cmath>

namespace Zhifen {

// ==================== 建筑底图AI精简 ====================
SimplifyResult AISimplifyTool::analyzeAndSimplify(QGraphicsScene *scene,
    bool keepWalls, bool keepDoors, bool keepPipes, bool keepDimensions)
{
    SimplifyResult result;
    if (!scene) {
        result.report = "场景为空，无法精简";
        return result;
    }

    // 收集所有图层
    QMap<QString, int> layerEntityCount;
    QList<QGraphicsItem*> itemsToRemove;

    for (QGraphicsItem *item : scene->items()) {
        QString layer = "默认图层";
        if (item->data(0).isValid()) {
            layer = item->data(0).toString();
        }
        layerEntityCount[layer]++;
        result.totalEntities++;

        // 判断是否需要删除
        bool shouldRemove = false;
        QString layerType = classifyLayer(layer);
        
        if (layerType == "wall" && !keepWalls) shouldRemove = true;
        else if (layerType == "door" && !keepDoors) shouldRemove = true;
        else if (layerType == "pipe" && !keepPipes) shouldRemove = true;
        else if (layerType == "dimension" && !keepDimensions) shouldRemove = true;
        else if (layerType == "furniture" || layerType == "hatch" || layerType == "other") {
            shouldRemove = true; // 默认删除家具、填充、其他
        }

        if (shouldRemove) {
            itemsToRemove.append(item);
            result.removedEntities++;
            if (!result.removedLayerNames.contains(layer)) {
                result.removedLayerNames.append(layer);
            }
        } else {
            result.keptEntities++;
            if (!result.keptLayerNames.contains(layer)) {
                result.keptLayerNames.append(layer);
            }
        }
    }

    // 执行删除
    for (QGraphicsItem *item : itemsToRemove) {
        scene->removeItem(item);
        delete item;
    }

    result.totalLayers = layerEntityCount.size();
    result.keptLayers = result.keptLayerNames.size();
    result.removedLayers = result.removedLayerNames.size();
    result.success = true;

    result.report = QString("=== 底图AI精简报告 ===\n"
        "总图层数: %1\n"
        "保留图层: %2\n"
        "删除图层: %3\n"
        "总图元数: %4\n"
        "保留图元: %5\n"
        "删除图元: %6\n\n"
        "保留图层:\n%7\n\n"
        "删除图层:\n%8")
        .arg(result.totalLayers)
        .arg(result.keptLayers)
        .arg(result.removedLayers)
        .arg(result.totalEntities)
        .arg(result.keptEntities)
        .arg(result.removedEntities)
        .arg(result.keptLayerNames.join(", "))
        .arg(result.removedLayerNames.join(", "));

    return result;
}

QString AISimplifyTool::classifyLayer(const QString &layerName)
{
    if (isWallLayer(layerName)) return "wall";
    if (isDoorWindowLayer(layerName)) return "door";
    if (isPipeLayer(layerName)) return "pipe";
    if (isDimensionLayer(layerName)) return "dimension";
    if (isFurnitureLayer(layerName)) return "furniture";
    if (isHatchLayer(layerName)) return "hatch";
    return "other";
}

bool AISimplifyTool::isWallLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("wall") || name.contains("墙") || name.contains("柱") || 
           name.contains("column") || name.contains("结构") || name.contains("structure");
}

bool AISimplifyTool::isDoorWindowLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("door") || name.contains("窗") || name.contains("window") ||
           name.contains("门") || name.contains("洞口") || name.contains("opening");
}

bool AISimplifyTool::isPipeLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("pipe") || name.contains("管") || name.contains("弱电") ||
           name.contains("强电") || name.contains("电气") || name.contains("electric") ||
           name.contains("桥架") || name.contains("cable");
}

bool AISimplifyTool::isDimensionLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("dim") || name.contains("标注") || name.contains("尺寸") ||
           name.contains("axis") || name.contains("轴网") || name.contains("标高");
}

bool AISimplifyTool::isFurnitureLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("furniture") || name.contains("家具") || name.contains("洁具") ||
           name.contains("设备") || name.contains("equipment") || name.contains("桌椅");
}

bool AISimplifyTool::isHatchLayer(const QString &layerName)
{
    QString name = layerName.toLower();
    return name.contains("hatch") || name.contains("填充") || name.contains("图案") ||
           name.contains("solid") || name.contains("地坪");
}

// ==================== 自动布放建议 ====================
AutoPlaceResult AutoPlaceTool::calculate(QGraphicsScene *scene,
    qreal targetPower, qreal antennaGain, qreal txPower, qreal coverageRadius, const QString &band)
{
    AutoPlaceResult result;
    if (!scene) {
        result.report = "场景为空，无法计算";
        return result;
    }

    QRectF area = scene->itemsBoundingRect();
    if (area.isEmpty()) {
        result.report = "场景范围为空，请先导入底图或绘制轮廓";
        return result;
    }

    // 计算面积（假设比例1:100，即1像素=10mm=0.01m）
    qreal scale = 0.01; // 像素转米
    result.totalArea = area.width() * scale * area.height() * scale;

    // 计算天线数量
    result.suggestedAntennaCount = calculateAntennaCount(result.totalArea, coverageRadius, 0.2);

    // 生成网格布放位置
    qreal spacing = coverageRadius * 1.6; // 间距为覆盖半径的1.6倍（20%重叠）
    QList<QPointF> positions = generateGridPositions(area, spacing / scale);

    for (const QPointF &pos : positions) {
        AntennaSuggestion suggestion;
        suggestion.position = pos;
        suggestion.coverageRadius = coverageRadius;
        suggestion.estimatedPower = txPower - 5.0; // 估算经过功分器后的功率
        suggestion.antennaType = "全向吸顶天线";
        suggestion.reason = QString("网格布放点，覆盖半径%1米").arg(coverageRadius);
        result.suggestions.append(suggestion);
    }

    result.avgSpacing = spacing;
    result.estimatedCoverageRate = qMin(100.0, result.totalArea / (result.suggestedAntennaCount * coverageRadius * coverageRadius * 3.14159) * 100);

    result.success = true;
    result.report = QString("=== 自动布放建议报告 ===\n"
        "总面积: %1 平方米\n"
        "目标覆盖功率: %2 dBm\n"
        "天线增益: %3 dBi\n"
        "发射功率: %4 dBm\n"
        "单天线覆盖半径: %5 米\n"
        "建议天线数量: %6 个\n"
        "平均间距: %7 米\n"
        "估算覆盖率: %8%\n\n"
        "建议: 在标记位置放置全向吸顶天线，可根据实际墙体位置微调")
        .arg(result.totalArea, 0, 'f', 1)
        .arg(targetPower)
        .arg(antennaGain)
        .arg(txPower)
        .arg(coverageRadius)
        .arg(result.suggestedAntennaCount)
        .arg(result.avgSpacing, 0, 'f', 1)
        .arg(result.estimatedCoverageRate, 0, 'f', 1);

    return result;
}

qreal AutoPlaceTool::calculateCoverageArea(qreal radius)
{
    return 3.14159 * radius * radius;
}

int AutoPlaceTool::calculateAntennaCount(qreal area, qreal coverageRadius, qreal overlap)
{
    qreal singleArea = calculateCoverageArea(coverageRadius) * (1 - overlap);
    return qMax(1, (int)ceil(area / singleArea));
}

QList<QPointF> AutoPlaceTool::generateGridPositions(const QRectF &area, qreal spacing)
{
    QList<QPointF> positions;
    qreal x = area.left() + spacing / 2;
    while (x < area.right()) {
        qreal y = area.top() + spacing / 2;
        while (y < area.bottom()) {
            positions.append(QPointF(x, y));
            y += spacing;
        }
        x += spacing;
    }
    return positions;
}

// ==================== 设备材料估算 ====================
MaterialEstimateResult MaterialEstimateTool::estimate(qreal area,
    int floorCount, const QString &sceneType, const QString &band, bool include5G)
{
    MaterialEstimateResult result;
    result.totalArea = area;
    result.floorCount = floorCount;

    // 估算天线数量
    int antennaCount = estimateAntennaCount(area, sceneType);

    // 估算馈线长度
    qreal feederLength = estimateFeederLength(antennaCount, area);

    // 生成主材和辅材清单
    result.mainMaterials = generateMainMaterials(antennaCount, feederLength, band, include5G);
    result.auxMaterials = generateAuxMaterials(antennaCount, feederLength);

    // 计算费用
    for (const auto &item : result.mainMaterials) {
        result.mainMaterialCost += item.totalPrice;
    }
    for (const auto &item : result.auxMaterials) {
        result.auxMaterialCost += item.totalPrice;
    }
    result.totalCost = result.mainMaterialCost + result.auxMaterialCost;
    result.costPerSquareMeter = area > 0 ? result.totalCost / area : 0;

    result.success = true;
    result.report = QString("=== 设备材料估算报告 ===\n"
        "项目类型: %1\n"
        "总面积: %2 平方米\n"
        "楼层数: %3 层\n"
        "频段: %4%5\n\n"
        "估算天线数量: %6 个\n"
        "估算馈线长度: %7 米\n\n"
        "主材费用: ¥%8\n"
        "辅材费用: ¥%9\n"
        "总费用: ¥%10\n"
        "每平米造价: ¥%11\n\n"
        "注: 本估算基于行业平均指标，实际费用以设计方案和采购价格为准")
        .arg(sceneType)
        .arg(area, 0, 'f', 1)
        .arg(floorCount)
        .arg(band)
        .arg(include5G ? " + 5G" : "")
        .arg(antennaCount)
        .arg(feederLength, 0, 'f', 0)
        .arg(result.mainMaterialCost, 0, 'f', 2)
        .arg(result.auxMaterialCost, 0, 'f', 2)
        .arg(result.totalCost, 0, 'f', 2)
        .arg(result.costPerSquareMeter, 0, 'f', 2);

    return result;
}

qreal MaterialEstimateTool::getCostPerSquareMeter(const QString &sceneType, const QString &band)
{
    QMap<QString, qreal> costs;
    costs["商业综合体"] = 35.0;
    costs["写字楼"] = 28.0;
    costs["酒店"] = 32.0;
    costs["医院"] = 38.0;
    costs["学校"] = 25.0;
    costs["住宅"] = 20.0;
    costs["地下车库"] = 22.0;
    costs["交通枢纽"] = 40.0;
    return costs.value(sceneType, 30.0);
}

int MaterialEstimateTool::estimateAntennaCount(qreal area, const QString &sceneType)
{
    // 不同场景的天线密度（个/1000平米）
    QMap<QString, qreal> density;
    density["商业综合体"] = 3.5;
    density["写字楼"] = 2.5;
    density["酒店"] = 3.0;
    density["医院"] = 3.2;
    density["学校"] = 2.0;
    density["住宅"] = 1.5;
    density["地下车库"] = 2.0;
    density["交通枢纽"] = 4.0;
    qreal d = density.value(sceneType, 2.5);
    return qMax(1, (int)ceil(area * d / 1000));
}

qreal MaterialEstimateTool::estimateFeederLength(int antennaCount, qreal area)
{
    // 经验公式：每个天线平均馈线长度 = 面积/天线数 * 0.8 + 10米
    qreal avgLength = antennaCount > 0 ? (area / antennaCount) * 0.8 + 10 : 20;
    return antennaCount * avgLength;
}

QList<MaterialEstimateItem> MaterialEstimateTool::generateMainMaterials(int antennaCount,
    qreal feederLength, const QString &band, bool include5G)
{
    QList<MaterialEstimateItem> items;

    // 信源
    MaterialEstimateItem source;
    source.category = "信源";
    source.name = "微基站";
    source.model = band + " 频段";
    source.quantity = 1;
    source.unit = "台";
    source.unitPrice = 8000;
    source.totalPrice = source.quantity * source.unitPrice;
    source.remark = "根据实际容量需求调整";
    items.append(source);

    // 天线
    MaterialEstimateItem antenna;
    antenna.category = "天线";
    antenna.name = "全向吸顶天线";
    antenna.model = "800-2700MHz";
    antenna.quantity = antennaCount;
    antenna.unit = "个";
    antenna.unitPrice = 80;
    antenna.totalPrice = antenna.quantity * antenna.unitPrice;
    items.append(antenna);

    // 功分器
    MaterialEstimateItem splitter2;
    splitter2.category = "器件";
    splitter2.name = "二功分器";
    splitter2.model = "800-2700MHz";
    splitter2.quantity = ceil(antennaCount * 0.4);
    splitter2.unit = "个";
    splitter2.unitPrice = 45;
    splitter2.totalPrice = splitter2.quantity * splitter2.unitPrice;
    items.append(splitter2);

    MaterialEstimateItem splitter3;
    splitter3.category = "器件";
    splitter3.name = "三功分器";
    splitter3.model = "800-2700MHz";
    splitter3.quantity = ceil(antennaCount * 0.2);
    splitter3.unit = "个";
    splitter3.unitPrice = 55;
    splitter3.totalPrice = splitter3.quantity * splitter3.unitPrice;
    items.append(splitter3);

    // 耦合器
    MaterialEstimateItem coupler;
    coupler.category = "器件";
    coupler.name = "耦合器";
    coupler.model = "5/10/15dB";
    coupler.quantity = ceil(antennaCount * 0.3);
    coupler.unit = "个";
    coupler.unitPrice = 50;
    coupler.totalPrice = coupler.quantity * coupler.unitPrice;
    items.append(coupler);

    // 馈线
    MaterialEstimateItem feeder;
    feeder.category = "馈线";
    feeder.name = "1/2\"馈线";
    feeder.model = "阻燃";
    feeder.quantity = feederLength;
    feeder.unit = "米";
    feeder.unitPrice = 12;
    feeder.totalPrice = feeder.quantity * feeder.unitPrice;
    items.append(feeder);

    // 5G设备（可选）
    if (include5G) {
        MaterialEstimateItem pRRU;
        pRRU.category = "5G设备";
        pRRU.name = "pRRU";
        pRRU.model = "5G 数字化";
        pRRU.quantity = ceil(antennaCount * 0.3);
        pRRU.unit = "台";
        pRRU.unitPrice = 3500;
        pRRU.totalPrice = pRRU.quantity * pRRU.unitPrice;
        pRRU.remark = "数字化室分设备";
        items.append(pRRU);
    }

    return items;
}

QList<MaterialEstimateItem> MaterialEstimateTool::generateAuxMaterials(int antennaCount,
    qreal feederLength)
{
    QList<MaterialEstimateItem> items;

    // 接头
    MaterialEstimateItem connector;
    connector.category = "辅材";
    connector.name = "N型接头";
    connector.model = "公头";
    connector.quantity = antennaCount * 2 + 20;
    connector.unit = "个";
    connector.unitPrice = 8;
    connector.totalPrice = connector.quantity * connector.unitPrice;
    items.append(connector);

    // 跳线
    MaterialEstimateItem jumper;
    jumper.category = "辅材";
    jumper.name = "跳线";
    jumper.model = "1/2\" 3米";
    jumper.quantity = antennaCount;
    jumper.unit = "条";
    jumper.unitPrice = 25;
    jumper.totalPrice = jumper.quantity * jumper.unitPrice;
    items.append(jumper);

    // 吊牌
    MaterialEstimateItem tag;
    tag.category = "辅材";
    tag.name = "吊牌";
    tag.model = "PVC";
    tag.quantity = antennaCount * 2;
    tag.unit = "个";
    tag.unitPrice = 1;
    tag.totalPrice = tag.quantity * tag.unitPrice;
    items.append(tag);

    // 扎带
    MaterialEstimateItem tie;
    tie.category = "辅材";
    tie.name = "扎带";
    tie.model = "4*200mm";
    tie.quantity = ceil(feederLength / 2);
    tie.unit = "包";
    tie.unitPrice = 15;
    tie.totalPrice = tie.quantity * tie.unitPrice;
    items.append(tie);

    // 防水胶带
    MaterialEstimateItem tape;
    tape.category = "辅材";
    tape.name = "防水胶带";
    tape.model = "3M";
    tape.quantity = ceil(antennaCount / 10.0);
    tape.unit = "卷";
    tape.unitPrice = 20;
    tape.totalPrice = tape.quantity * tape.unitPrice;
    items.append(tape);

    return items;
}

} // namespace Zhifen
