#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>
#include <QList>

class CadItem;
class CadScene;

struct LayerInfo {
    QString name;
    QColor color;
    bool visible = true;
    bool locked = false;
    bool frozen = false;
    bool plot = true;
    bool newViewport = true;      // 新视口冻结
    bool currentViewport = false; // 当前视口冻结
    QString lineType = "Continuous";
    qreal lineWidth = 0.25;
    QString plotStyle = "Normal"; // 打印样式
    QString description = "";     // 描述
    QString group = "";           // 所属图层组
    int transparency = 0;         // 透明度(0-90)
};

// 图层组
struct LayerGroup {
    QString name;
    QString description;
    bool expanded = true;
    QList<QString> layerNames;
};

// 图层状态
struct LayerState {
    QString name;
    QString description;
    QMap<QString, LayerInfo> layers;
};

class Document : public QObject
{
    Q_OBJECT
public:
    explicit Document(QObject *parent = nullptr);

    // 图层管理
    void addLayer(const QString &name, const QColor &color = Qt::white);
    bool removeLayer(const QString &name);
    LayerInfo* getLayer(const QString &name);
    QStringList getAllLayerNames() const;
    QList<LayerInfo> getAllLayers() const;
    void setCurrentLayer(const QString &name);
    QString currentLayer() const { return m_currentLayer; }
    void resetToDefaultLayers();
    // 图层状态管理
    void saveLayerState(const QString &name, const QString &description = "");
    bool restoreLayerState(const QString &name);
    bool deleteLayerState(const QString &name);
    QStringList getAllLayerStateNames() const;
    LayerState* getLayerState(const QString &name);
    // 图层组管理
    void addLayerGroup(const QString &name, const QString &description = "");
    bool removeLayerGroup(const QString &name);
    void addLayerToGroup(const QString &layerName, const QString &groupName);
    void removeLayerFromGroup(const QString &layerName, const QString &groupName);
    QStringList getAllGroupNames() const;
    LayerGroup* getLayerGroup(const QString &name);
    // 图层过滤
    QStringList filterLayers(const QString &keyword, bool showHidden = false) const;
    // 图层操作
    void setLayerVisible(const QString &name, bool visible);
    void setLayerLocked(const QString &name, bool locked);
    void setLayerFrozen(const QString &name, bool frozen);
    void setLayerPlot(const QString &name, bool plot);
    void setLayerColor(const QString &name, const QColor &color);
    void setLayerLineType(const QString &name, const QString &lineType);
    void setLayerLineWidth(const QString &name, qreal lineWidth);
    void isolateLayer(const QString &name);
    void unisolateLayer();
    void turnAllLayersOn();
    void freezeAllLayersExcept(const QString &name);
    void lockAllLayersExcept(const QString &name);

    // 文档属性
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path) { m_filePath = path; }
    bool isModified() const { return m_modified; }
    void setModified(bool modified) { m_modified = modified; }

    // 场景
    CadScene* scene() const { return m_scene; }
    void setScene(CadScene *scene) { m_scene = scene; }

    // 统计
    int entityCount() const;

signals:
    void layerChanged();
    void modifiedChanged(bool modified);

private:
    QString m_name = "未命名";
    QMap<QString, LayerState> m_layerStates;
    QMap<QString, LayerGroup> m_layerGroups;
    QStringList m_isolatedLayers;
    QString m_filePath;
    QString m_currentLayer = "0";
    QMap<QString, LayerInfo> m_layers;
    CadScene *m_scene = nullptr;
    bool m_modified = false;

    void initDefaultLayers();
};

#endif // DOCUMENT_H
