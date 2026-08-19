#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include "zf_types.h"
#include "zf_error.h"
#include "../mode_control/mode_control_layer.h"
#include "../device/device_library.h"
#include "../undo/undo_redo_stack.h"

namespace zf {

class ZfSnapEngine {
public:
    bool enabled{true};
    bool gridSnap{true};
    bool endpointSnap{true};
    bool portSemanticSnap{false};
    double tolerancePx{8.0};
    double gridSize{100.0};

    Point2D applySnap(Point2D rawPt) const {
        if (!enabled) return rawPt;
        Point2D pt = rawPt;
        if (gridSnap && gridSize > 0) {
            pt.x = std::round(pt.x / gridSize) * gridSize;
            pt.y = std::round(pt.y / gridSize) * gridSize;
        }
        return pt;
    }
    void setPortSnapEnabled(bool enable) { portSemanticSnap = enable; }
    bool portSnapEnabled() const { return portSemanticSnap; }
};

class ZfToolBase {
public:
    virtual ~ZfToolBase() = default;
    virtual std::string toolName() const = 0;
    virtual ToolType toolType() const = 0;
    virtual void enterTool() { m_active = true; }
    virtual void leaveTool() { cleanupSemanticSnap(); m_active = false; }
    virtual void onMousePress(Point2D) {}
    virtual void onMouseMove(Point2D) {}
    virtual void onKeyPress(int) {}
    bool isActive() const { return m_active; }

    void setContext(ZfSnapEngine* snap, ModeManager* mode, DeviceLibrary* lib, UndoRedoDoubleStack* undo) {
        m_snapEngine = snap;
        m_modeMgr = mode;
        m_deviceLib = lib;
        m_undoStack = undo;
    }

protected:
    bool m_active{false};
    ZfSnapEngine* m_snapEngine{nullptr};
    ModeManager* m_modeMgr{nullptr};
    DeviceLibrary* m_deviceLib{nullptr};
    UndoRedoDoubleStack* m_undoStack{nullptr};

    void cleanupSemanticSnap() {
        if (m_snapEngine) m_snapEngine->setPortSnapEnabled(false);
    }
};

class ZfSelectTool : public ZfToolBase {
public:
    std::string toolName() const override { return "选择"; }
    ToolType toolType() const override { return ToolType::SELECT; }
};

class ZfPlaceDeviceTool : public ZfToolBase {
public:
    std::string toolName() const override { return "放置器件"; }
    ToolType toolType() const override { return ToolType::PLACE_DEVICE; }

    void enterTool() override {
        ZfToolBase::enterTool();
        if (m_snapEngine) m_snapEngine->setPortSnapEnabled(true);
    }
    void leaveTool() override {
        ZfToolBase::leaveTool();
        m_continuousPlace = false;
    }

    void setCurrentModelId(const std::string& id) { m_currentModelId = id; }
    std::string currentModelId() const { return m_currentModelId; }
    void setContinuousPlace(bool v) { m_continuousPlace = v; }
    bool continuousPlace() const { return m_continuousPlace; }

    DeviceInstance placeDevice(Point2D pos) {
        DeviceInstance inst;
        inst.instanceId = "DEV_" + std::to_string(std::rand());
        inst.modelId = m_currentModelId;
        inst.position = pos;
        inst.status = LinkStatus::NOT_CALCULATED;
        if (m_modeMgr && m_modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE)
            inst.sketchPlaceholder = true;
        return inst;
    }

private:
    std::string m_currentModelId;
    bool m_continuousPlace{false};
};

class ZfDrawCableTool : public ZfToolBase {
public:
    std::string toolName() const override { return "绘制馈线"; }
    ToolType toolType() const override { return ToolType::DRAW_CABLE; }

    void enterTool() override {
        ZfToolBase::enterTool();
        if (m_snapEngine) m_snapEngine->setPortSnapEnabled(true);
        m_drawing = false;
        m_routePoints.clear();
    }
    void leaveTool() override {
        ZfToolBase::leaveTool();
        m_drawing = false;
        m_routePoints.clear();
    }

    void setCableModelId(const std::string& id) { m_cableModelId = id; }

    void addPoint(Point2D pt) {
        m_routePoints.push_back(pt);
        m_drawing = true;
    }

    CableSegment finishCable() {
        CableSegment seg;
        seg.segmentId = "CAB_" + std::to_string(std::rand());
        seg.modelId = m_cableModelId;
        seg.routePoints = m_routePoints;
        m_drawing = false;
        m_routePoints.clear();
        return seg;
    }

private:
    std::string m_cableModelId{"CABLE_1_2"};
    std::vector<Point2D> m_routePoints;
    bool m_drawing{false};
};

class ZfEditTool : public ZfToolBase {
public:
    std::string toolName() const override { return "编辑复制"; }
    ToolType toolType() const override { return ToolType::EDIT; }

    void setCopyMode(CopyDuplicateMode mode) { m_copyMode = mode; }
    CopyDuplicateMode copyMode() const { return m_copyMode; }

    DeviceInstance makeLightCopy(const DeviceInstance& src, Point2D offset) {
        DeviceInstance dst;
        dst.instanceId = "DEV_" + std::to_string(std::rand());
        dst.modelId = src.modelId;
        dst.position.x = src.position.x + offset.x;
        dst.position.y = src.position.y + offset.y;
        dst.rotation = src.rotation;
        dst.elevation_m = src.elevation_m;
        dst.floorId = src.floorId;
        dst.label.clear();
        dst.userNote.clear();
        dst.cellId.clear();
        dst.sourceId.clear();
        dst.inputPower_dBm.clear();
        dst.outputPower_dBm.clear();
        dst.connections.clear();
        dst.status = LinkStatus::NOT_CALCULATED;
        dst.sketchPlaceholder = true;
        return dst;
    }

    DeviceInstance makeFullCopy(const DeviceInstance& src, Point2D offset) {
        DeviceInstance dst = src;
        dst.instanceId = "DEV_" + std::to_string(std::rand());
        dst.position.x = src.position.x + offset.x;
        dst.position.y = src.position.y + offset.y;
        return dst;
    }

    void doDuplicate(std::vector<DeviceInstance>& devices, const std::vector<std::string>& ids, Point2D offset) {
        for (const auto& id : ids) {
            for (const auto& dev : devices) {
                if (dev.instanceId == id) {
                    DeviceInstance newInst = (m_copyMode == CopyDuplicateMode::LIGHT_COPY)
                        ? makeLightCopy(dev, offset) : makeFullCopy(dev, offset);
                    if (m_modeMgr) {
                        AuditEntry e;
                        e.actionType = (m_copyMode == CopyDuplicateMode::LIGHT_COPY) ? "copy_light" : "copy_complete";
                        e.targetObjectIds.push_back(newInst.instanceId);
                        m_modeMgr->appendAuditEntry(e);
                    }
                    devices.push_back(newInst);
                    break;
                }
            }
        }
    }

private:
    CopyDuplicateMode m_copyMode{CopyDuplicateMode::LIGHT_COPY};
};

class ToolManager {
public:
    ToolManager() { registerBuiltinTools(); }

    void init(ZfSnapEngine* snap, ModeManager* mode, DeviceLibrary* lib, UndoRedoDoubleStack* undo) {
        for (auto& pair : m_tools)
            pair.second->setContext(snap, mode, lib, undo);
    }

    void activateTool(ToolType type) {
        if (m_activeTool) m_activeTool->leaveTool();
        auto it = m_tools.find(type);
        if (it == m_tools.end()) return;
        m_activeTool = it->second.get();
        m_activeTool->enterTool();
    }

    ZfToolBase* activeTool() { return m_activeTool; }
    void escapeCurrentTool() {
        if (m_activeTool && m_activeTool->toolType() != ToolType::SELECT)
            activateTool(ToolType::SELECT);
    }

private:
    std::unordered_map<ToolType, std::unique_ptr<ZfToolBase>> m_tools;
    ZfToolBase* m_activeTool{nullptr};

    void registerBuiltinTools() {
        m_tools[ToolType::SELECT] = std::make_unique<ZfSelectTool>();
        m_tools[ToolType::PLACE_DEVICE] = std::make_unique<ZfPlaceDeviceTool>();
        m_tools[ToolType::DRAW_CABLE] = std::make_unique<ZfDrawCableTool>();
        m_tools[ToolType::EDIT] = std::make_unique<ZfEditTool>();
    }
};

} // namespace zf
