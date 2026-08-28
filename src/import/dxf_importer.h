#ifndef DXF_IMPORTER_H
#define DXF_IMPORTER_H

#include <QString>
#include <QList>
#include <QPointF>
#include <QColor>
#include <QGraphicsScene>

namespace Zhifen {

// DXF图元类型
enum DxfEntityType {
    Dxf_Line = 0,
    Dxf_LwPolyline = 1,
    Dxf_Circle = 2,
    Dxf_Arc = 3,
    Dxf_Text = 4,
    Dxf_Unknown = 99
};

// DXF图元
struct DxfEntity {
    DxfEntityType type = Dxf_Unknown;
    QString layer;
    QColor color = Qt::black;
    QPointF start;
    QPointF end;
    QPointF center;
    qreal radius = 0;
    qreal startAngle = 0;
    qreal endAngle = 0;
    QString text;
    qreal textHeight = 2.5;
    QList<QPointF> vertices;  // LWPOLYLINE顶点
    bool closed = false;
};

// 图层信息
struct LayerInfo {
    QString name;
    QColor color = Qt::black;
    bool visible = true;
    bool locked = false;
    int entityCount = 0;
};

// DXF导入结果
struct DxfImportResult {
    bool success = false;
    QList<DxfEntity> entities;
    QList<LayerInfo> layers;
    QStringList errors;
    QStringList warnings;
};

// 底图精简模式
enum SimplifyMode {
    Simplify_None = 0,       // 不精简，全部导入
    Simplify_Basic = 1,      // 基础精简：保留墙体/门窗/管线
    Simplify_Aggressive = 2  // 深度精简：仅保留墙体
};

// DXF导入器
class DxfImporter
{
public:
    DxfImporter();
    ~DxfImporter();

    // 从文件导入DXF
    DxfImportResult importFromFile(const QString &filePath, SimplifyMode mode = Simplify_Basic);

    // 将DXF图元渲染到场景
    void renderToScene(const DxfImportResult &result, QGraphicsScene *scene, bool lockBottom = true);

    // 获取图层列表
    QList<LayerInfo> layers() const { return m_layers; }

    // 设置图层可见性
    void setLayerVisible(const QString &layerName, bool visible);

    // 设置图层锁定
    void setLayerLocked(const QString &layerName, bool locked);

private:
    QList<LayerInfo> m_layers;
    QMap<QString, bool> m_layerVisible;
    QMap<QString, bool> m_layerLocked;

    // 解析DXF文件
    bool parseDxf(const QString &filePath, DxfImportResult &result);

    // 精简过滤
    void simplify(DxfImportResult &result, SimplifyMode mode);

    // 判断图层是否需要保留
    bool shouldKeepLayer(const QString &layerName, SimplifyMode mode);

    // 图层分类
    static QString classifyLayer(const QString &layerName);

    // 判断是否为墙体图层
    static bool isWallLayer(const QString &layerName);

    // 判断是否为门窗图层
    static bool isDoorWindowLayer(const QString &layerName);

    // 判断是否为管线图层
    static bool isPipeLayer(const QString &layerName);

    // 判断是否为标注图层
    static bool isDimensionLayer(const QString &layerName);

    // 判断是否为填充图层
    static bool isHatchLayer(const QString &layerName);

    // 判断是否为家具图层
    static bool isFurnitureLayer(const QString &layerName);
};

} // namespace Zhifen

#endif // DXF_IMPORTER_H
