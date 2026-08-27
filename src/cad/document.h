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
    QString lineType = "Continuous";
    qreal lineWidth = 0.25;
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
    QString m_filePath;
    QString m_currentLayer = "0";
    QMap<QString, LayerInfo> m_layers;
    CadScene *m_scene = nullptr;
    bool m_modified = false;

    void initDefaultLayers();
};

#endif // DOCUMENT_H
