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

// 图层状态管理
void Document::saveLayerState(const QString &name, const QString &description)
{
    LayerState state;
    state.name = name;
    state.description = description;
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        state.layers[it.key()] = it.value();
    }
    m_layerStates[name] = state;
}

bool Document::restoreLayerState(const QString &name)
{
    if (!m_layerStates.contains(name)) return false;
    LayerState state = m_layerStates[name];
    for (auto it = state.layers.begin(); it != state.layers.end(); ++it) {
        if (m_layers.contains(it.key())) {
            m_layers[it.key()] = it.value();
        }
    }
    emit layerChanged();
    return true;
}

bool Document::deleteLayerState(const QString &name)
{
    return m_layerStates.remove(name) > 0;
}

QStringList Document::getAllLayerStateNames() const
{
    return m_layerStates.keys();
}

LayerState* Document::getLayerState(const QString &name)
{
    if (m_layerStates.contains(name)) {
        return &m_layerStates[name];
    }
    return nullptr;
}

// 图层组管理
void Document::addLayerGroup(const QString &name, const QString &description)
{
    if (m_layerGroups.contains(name)) return;
    LayerGroup group;
    group.name = name;
    group.description = description;
    m_layerGroups[name] = group;
}

bool Document::removeLayerGroup(const QString &name)
{
    if (!m_layerGroups.contains(name)) return false;
    // 从组中移除所有图层
    for (auto &layerName : m_layerGroups[name].layerNames) {
        if (m_layers.contains(layerName)) {
            m_layers[layerName].group = "";
        }
    }
    m_layerGroups.remove(name);
    return true;
}

void Document::addLayerToGroup(const QString &layerName, const QString &groupName)
{
    if (!m_layers.contains(layerName) || !m_layerGroups.contains(groupName)) return;
    // 从其他组移除
    for (auto &group : m_layerGroups) {
        group.layerNames.removeAll(layerName);
    }
    m_layerGroups[groupName].layerNames.append(layerName);
    m_layers[layerName].group = groupName;
    emit layerChanged();
}

void Document::removeLayerFromGroup(const QString &layerName, const QString &groupName)
{
    if (!m_layerGroups.contains(groupName)) return;
    m_layerGroups[groupName].layerNames.removeAll(layerName);
    if (m_layers.contains(layerName)) {
        m_layers[layerName].group = "";
    }
    emit layerChanged();
}

QStringList Document::getAllGroupNames() const
{
    return m_layerGroups.keys();
}

LayerGroup* Document::getLayerGroup(const QString &name)
{
    if (m_layerGroups.contains(name)) {
        return &m_layerGroups[name];
    }
    return nullptr;
}

// 图层过滤
QStringList Document::filterLayers(const QString &keyword, bool showHidden) const
{
    QStringList result;
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (!showHidden && !it.value().visible) continue;
        if (keyword.isEmpty() || it.key().contains(keyword, Qt::CaseInsensitive)) {
            result.append(it.key());
        }
    }
    return result;
}

// 图层操作
void Document::setLayerVisible(const QString &name, bool visible)
{
    if (m_layers.contains(name)) {
        m_layers[name].visible = visible;
        emit layerChanged();
    }
}

void Document::setLayerLocked(const QString &name, bool locked)
{
    if (m_layers.contains(name)) {
        m_layers[name].locked = locked;
        emit layerChanged();
    }
}

void Document::setLayerFrozen(const QString &name, bool frozen)
{
    if (m_layers.contains(name)) {
        m_layers[name].frozen = frozen;
        emit layerChanged();
    }
}

void Document::setLayerPlot(const QString &name, bool plot)
{
    if (m_layers.contains(name)) {
        m_layers[name].plot = plot;
        emit layerChanged();
    }
}

void Document::setLayerColor(const QString &name, const QColor &color)
{
    if (m_layers.contains(name)) {
        m_layers[name].color = color;
        emit layerChanged();
    }
}

void Document::setLayerLineType(const QString &name, const QString &lineType)
{
    if (m_layers.contains(name)) {
        m_layers[name].lineType = lineType;
        emit layerChanged();
    }
}

void Document::setLayerLineWidth(const QString &name, qreal lineWidth)
{
    if (m_layers.contains(name)) {
        m_layers[name].lineWidth = lineWidth;
        emit layerChanged();
    }
}

void Document::isolateLayer(const QString &name)
{
    m_isolatedLayers.clear();
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (it.key() != name && it.value().visible) {
            m_isolatedLayers.append(it.key());
            it.value().visible = false;
        }
    }
    emit layerChanged();
}

void Document::unisolateLayer()
{
    for (auto &name : m_isolatedLayers) {
        if (m_layers.contains(name)) {
            m_layers[name].visible = true;
        }
    }
    m_isolatedLayers.clear();
    emit layerChanged();
}

void Document::turnAllLayersOn()
{
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        it.value().visible = true;
        it.value().frozen = false;
    }
    emit layerChanged();
}

void Document::freezeAllLayersExcept(const QString &name)
{
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (it.key() != name) {
            it.value().frozen = true;
        }
    }
    emit layerChanged();
}

void Document::lockAllLayersExcept(const QString &name)
{
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if (it.key() != name) {
            it.value().locked = true;
        }
    }
    emit layerChanged();
}
