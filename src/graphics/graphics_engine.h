#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include "zf_types.h"
#include "zf_error.h"

namespace zf {

enum ItemType {
    ITEM_UNKNOWN = 0,
    ITEM_WALL = 1,
    ITEM_ANTENNA = 10,
    ITEM_SPLITTER = 11,
    ITEM_COUPLER = 12,
    ITEM_COMBINER = 13,
    ITEM_SOURCE = 14,
    ITEM_CABLE = 20,
    ITEM_LABEL = 30,
    ITEM_IMAGE = 40
};

class ZfItemBase {
public:
    virtual ~ZfItemBase() = default;
    std::string itemId;
    std::string layerName;
    bool visible{true};
    bool selectable{true};
    bool locked{false};
    Point2D position;
    double rotation{0.0};

    virtual int itemType() const = 0;
    virtual Rect2D boundingRect() const = 0;
    virtual bool contains(Point2D pt) const {
        Rect2D b = boundingRect();
        return pt.x >= b.origin.x && pt.x <= b.origin.x + b.size.w
            && pt.y >= b.origin.y && pt.y <= b.origin.y + b.size.h;
    }
    virtual void move(Point2D delta) {
        position.x += delta.x;
        position.y += delta.y;
    }
};

class ZfScene {
public:
    ZfScene() = default;

    ZfItemBase* addItem(std::unique_ptr<ZfItemBase> item) {
        if (!item) return nullptr;
        ZfItemBase* ptr = item.get();
        m_items.push_back(std::move(item));
        return ptr;
    }

    void removeItem(const std::string& itemId) {
        auto it = std::remove_if(m_items.begin(), m_items.end(),
            [&](const std::unique_ptr<ZfItemBase>& it) { return it->itemId == itemId; });
        if (it != m_items.end()) {
            m_items.erase(it, m_items.end());
            deselectItem(itemId);
        }
    }

    void clear() { m_items.clear(); m_selection.clear(); }

    ZfItemBase* findItemById(const std::string& itemId) {
        for (auto& it : m_items)
            if (it->itemId == itemId) return it.get();
        return nullptr;
    }

    ZfItemBase* itemAt(Point2D pos) {
        for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
            if (!(*it)->visible || !(*it)->selectable) continue;
            if ((*it)->contains(pos)) return it->get();
        }
        return nullptr;
    }

    size_t itemCount() const { return m_items.size(); }

    void selectItem(const std::string& id) {
        if (std::find(m_selection.begin(), m_selection.end(), id) == m_selection.end())
            m_selection.push_back(id);
    }
    void deselectItem(const std::string& id) {
        auto it = std::remove(m_selection.begin(), m_selection.end(), id);
        if (it != m_selection.end()) m_selection.erase(it, m_selection.end());
    }
    void clearSelection() { m_selection.clear(); }
    std::vector<std::string> selectedItems() const { return m_selection; }

private:
    std::vector<std::unique_ptr<ZfItemBase>> m_items;
    std::vector<std::string> m_selection;
};

class ZfView {
public:
    explicit ZfView(ZfScene* scene) : m_scene(scene) {}

    void zoomIn(double f = 1.2) { m_zoom *= f; }
    void zoomOut(double f = 1.2) { m_zoom /= f; }
    void pan(Point2D delta) { m_offset.x += delta.x; m_offset.y += delta.y; }
    double zoomLevel() const { return m_zoom; }
    Point2D viewCenter() const { return m_offset; }

    Point2D viewToScene(Point2D v) const {
        return {(v.x - m_offset.x) / m_zoom, (v.y - m_offset.y) / m_zoom};
    }
    Point2D sceneToView(Point2D s) const {
        return {s.x * m_zoom + m_offset.x, s.y * m_zoom + m_offset.y};
    }

private:
    ZfScene* m_scene{nullptr};
    double m_zoom{1.0};
    Point2D m_offset;
};

struct LayerInfo {
    std::string name;
    std::string displayName;
    Color color;
    bool visible{true};
    bool locked{false};
    bool printable{true};
    int zIndex{0};
};

class ZfLayerManager {
public:
    ZfLayerManager() = default;

    void createLayer(const std::string& name, const std::string& displayName, Color color) {
        LayerInfo info;
        info.name = name;
        info.displayName = displayName;
        info.color = color;
        m_layers[name] = std::move(info);
    }

    void setLayerVisible(const std::string& name, bool v) {
        auto it = m_layers.find(name);
        if (it != m_layers.end()) it->second.visible = v;
    }
    void setLayerLocked(const std::string& name, bool l) {
        auto it = m_layers.find(name);
        if (it != m_layers.end()) it->second.locked = l;
    }
    bool isLayerVisible(const std::string& name) const {
        auto it = m_layers.find(name);
        return it != m_layers.end() ? it->second.visible : true;
    }
    bool isLayerLocked(const std::string& name) const {
        auto it = m_layers.find(name);
        return it != m_layers.end() ? it->second.locked : false;
    }
    int layerCount() const { return static_cast<int>(m_layers.size()); }

    void initDefaultLayers() {
        createLayer("TY_WALL", "墙体", Color{128,128,128,255});
        createLayer("TY_ANTENNA", "天线", Color{255,0,0,255});
        createLayer("TY_SPLITTER", "功分器", Color{255,255,0,255});
        createLayer("TY_COUPLER", "耦合器", Color{0,255,0,255});
        createLayer("TY_CABLE_7_8", "7/8馈线", Color{255,0,0,255});
        createLayer("TY_CABLE_1_2", "1/2馈线", Color{255,0,0,255});
        createLayer("TY_SOURCE", "信源", Color{0,0,255,255});
        createLayer("TY_LABEL", "标注", Color{0,0,0,255});
        createLayer("TY_BACKGROUND", "底图", Color{255,255,255,255});
    }

private:
    std::unordered_map<std::string, LayerInfo> m_layers;
};

} // namespace zf
