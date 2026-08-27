#include "document.h"
#include "cadscene.h"
#include "caditem.h"
#include <QGraphicsItem>

Document::Document(QObject *parent)
    : QObject(parent)
{
    initDefaultLayers();
}

void Document::initDefaultLayers()
{
    addLayer("0", QColor(255, 255, 255));
    addLayer("DEFPOINTS", QColor(255, 255, 255));
    addLayer("墙体", QColor(128, 128, 128));
    addLayer("门窗", QColor(255, 255, 0));
    addLayer("天线", QColor(255, 0, 0));
    addLayer("器件", QColor(0, 0, 255));
    addLayer("馈线", QColor(255, 255, 255));
    addLayer("光纤", QColor(255, 165, 0));
    addLayer("标注", QColor(0, 255, 255));
    addLayer("文字", QColor(0, 255, 0));
    m_currentLayer = "0";
}

void Document::resetToDefaultLayers()
{
    m_layers.clear();
    initDefaultLayers();
    emit layerChanged();
}

void Document::addLayer(const QString &name, const QColor &color)
{
    if (m_layers.contains(name)) return;
    LayerInfo info;
    info.name = name;
    info.color = color;
    m_layers[name] = info;
    emit layerChanged();
}

bool Document::removeLayer(const QString &name)
{
    if (name == "0" || name == "DEFPOINTS") return false;
    if (!m_layers.contains(name)) return false;
    m_layers.remove(name);
    if (m_currentLayer == name) m_currentLayer = "0";
    emit layerChanged();
    return true;
}

LayerInfo* Document::getLayer(const QString &name)
{
    if (!m_layers.contains(name)) return nullptr;
    return &m_layers[name];
}

QStringList Document::getAllLayerNames() const
{
    return m_layers.keys();
}

QList<LayerInfo> Document::getAllLayers() const
{
    return m_layers.values();
}

void Document::setCurrentLayer(const QString &name)
{
    if (m_layers.contains(name)) {
        m_currentLayer = name;
        emit layerChanged();
    }
}

int Document::entityCount() const
{
    if (!m_scene) return 0;
    int count = 0;
    for (auto item : m_scene->items()) {
        if (dynamic_cast<CadItem*>(item)) count++;
    }
    return count;
}
