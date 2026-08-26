#pragma once

#include <string>
#include <memory>
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {
using json = nlohmann::json;

class ProjectIO {
public:
    ProjectIO() = default;

    std::unique_ptr<Project> loadProject(const std::string& filePath) {
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) return nullptr;
        try {
            json j;
            ifs >> j;
            auto proj = std::make_unique<Project>();
            deserializeProject(j, proj.get());
            return proj;
        } catch (...) {
            return nullptr;
        }
    }

    int saveProject(const std::string& filePath, const Project* project) {
        if (!project) return ZF_ERR_ARG;
        try {
            json j;
            serializeProject(project, j);
            std::ofstream ofs(filePath);
            if (!ofs.is_open()) return ZF_ERR_IO_OPEN_FAILED;
            ofs << std::setw(2) << j << std::endl;
            return ZF_ERR_OK;
        } catch (...) {
            return ZF_ERR_IO_WRITE_FAILED;
        }
    }

    static constexpr int FILE_VERSION = 310;

private:
    void serializeProject(const Project* p, json& j) {
        j["file_version"] = FILE_VERSION;
        j["projectId"] = p->projectId;
        j["projectName"] = p->projectName;
        j["operatorName"] = p->operatorName;
        j["scenarioType"] = p->scenarioType;
        j["designer"] = p->designer;
        j["description"] = p->description;
        j["version"] = p->version;
        j["coverageThreshold_dBm"] = p->coverageThreshold_dBm;
        j["coverageTarget"] = p->coverageTarget;
        j["maxAntennaPower_dBm"] = p->maxAntennaPower_dBm;
        j["maxFeederLoss_dB"] = p->maxFeederLoss_dB;
        j["minSINR_dB"] = p->minSINR_dB;
        j["globalWorkMode"] = static_cast<int>(p->globalWorkMode);
        j["defaultCopyMode"] = static_cast<int>(p->defaultCopyMode);

        json floors = json::array();
        for (const auto& f : p->floors) {
            json fj;
            serializeFloor(f, fj);
            floors.push_back(fj);
        }
        j["floors"] = floors;

        // 图层
        json layers = json::array();
        for (const auto& l : p->layers) {
            json lj;
            lj["layerId"] = l.layerId;
            lj["name"] = l.name;
            lj["type"] = static_cast<int>(l.type);
            lj["color"] = l.color;
            lj["visible"] = l.visible;
            lj["locked"] = l.locked;
            lj["frozen"] = l.frozen;
            lj["lineWidth"] = l.lineWidth;
            lj["order"] = l.order;
            layers.push_back(lj);
        }
        j["layers"] = layers;

        // 打印窗口
        json printWins = json::array();
        for (const auto& pw : p->printWindows) {
            json pj;
            pj["name"] = pw.name;
            pj["minX"] = pw.minPt.x;
            pj["minY"] = pw.minPt.y;
            pj["maxX"] = pw.maxPt.x;
            pj["maxY"] = pw.maxPt.y;
            pj["paperSize"] = pw.paperSize;
            pj["scale"] = pw.scale;
            pj["color"] = pw.color;
            printWins.push_back(pj);
        }
        j["printWindows"] = printWins;

        // 系统图序列化
        json sysDiags = json::array();
        for (const auto& sd : p->systemDiagrams) {
            json sdj;
            sdj["diagramId"] = sd.diagramId;
            sdj["floorId"] = sd.floorId;
            json nodes = json::array();
            for (const auto& n : sd.nodes) {
                json nj;
                nj["nodeId"] = n.nodeId;
                nj["type"] = (int)n.type;
                nj["deviceInstanceId"] = n.deviceInstanceId;
                nj["layoutX"] = n.layoutPos.x;
                nj["layoutY"] = n.layoutPos.y;
                nj["label"] = n.label;
                nodes.push_back(nj);
            }
            sdj["nodes"] = nodes;
            json links = json::array();
            for (const auto& l : sd.links) {
                json lj;
                lj["linkId"] = l.linkId;
                lj["fromNodeId"] = l.fromNodeId;
                lj["toNodeId"] = l.toNodeId;
                lj["cableModelId"] = l.cableModelId;
                lj["length_m"] = l.length_m;
                lj["loss_dB"] = l.loss_dB;
                links.push_back(lj);
            }
            sdj["links"] = links;
            sysDiags.push_back(sdj);
        }
        j["systemDiagrams"] = sysDiags;

        serializeAudit(p->globalAuditLog, j["globalAuditLog"]);
    }

    void deserializeProject(const json& j, Project* p) {
        p->projectId = j.value("projectId", "");
        p->projectName = j.value("projectName", "未命名工程");
        p->operatorName = j.value("operatorName", "");
        p->scenarioType = j.value("scenarioType", "");
        p->designer = j.value("designer", "");
        p->description = j.value("description", "");
        p->version = j.value("version", 1);
        p->coverageThreshold_dBm = j.value("coverageThreshold_dBm", -75.0);
        p->coverageTarget = j.value("coverageTarget", 0.95);
        p->maxAntennaPower_dBm = j.value("maxAntennaPower_dBm", 15.0);
        p->maxFeederLoss_dB = j.value("maxFeederLoss_dB", 3.0);
        p->minSINR_dB = j.value("minSINR_dB", 0.0);
        p->globalWorkMode = static_cast<WorkMode>(j.value("globalWorkMode", 0));
        p->defaultCopyMode = static_cast<CopyDuplicateMode>(j.value("defaultCopyMode", 0));

        if (j.contains("floors") && j["floors"].is_array()) {
            for (const auto& fj : j["floors"]) {
                Floor f;
                deserializeFloor(fj, f);
                p->floors.push_back(std::move(f));
            }
        }
        // 自动排列楼层origin（如果都是0）
        bool allOriginZero = true;
        for (const auto& f : p->floors) {
            if (f.origin.x != 0 || f.origin.y != 0) { allOriginZero = false; break; }
        }
        if (allOriginZero) {
            for (size_t i = 0; i < p->floors.size(); i++) {
                p->floors[i].origin.x = i * 35000.0;
                p->floors[i].origin.y = 0;
            }
        }
        // 图层
        if (j.contains("layers") && j["layers"].is_array()) {
            for (const auto& lj : j["layers"]) {
                Layer l;
                l.layerId = lj.value("layerId", "");
                l.name = lj.value("name", "");
                l.type = static_cast<LayerType>(lj.value("type", 8));
                l.color = lj.value("color", 0xFFFFFF);
                l.visible = lj.value("visible", true);
                l.locked = lj.value("locked", false);
                l.frozen = lj.value("frozen", false);
                l.lineWidth = lj.value("lineWidth", 0.1);
                l.order = lj.value("order", 0);
                p->layers.push_back(l);
            }
        }
        if (p->layers.empty()) p->initDefaultLayers();
        // 打印窗口
        if (j.contains("printWindows") && j["printWindows"].is_array()) {
            for (const auto& pj : j["printWindows"]) {
                PrintWindow pw;
                pw.name = pj.value("name", "");
                pw.minPt.x = pj.value("minX", 0.0);
                pw.minPt.y = pj.value("minY", 0.0);
                pw.maxPt.x = pj.value("maxX", 0.0);
                pw.maxPt.y = pj.value("maxY", 0.0);
                pw.paperSize = pj.value("paperSize", "A1");
                pw.scale = pj.value("scale", 100.0);
                pw.color = pj.value("color", true);
                p->printWindows.push_back(pw);
            }
        }
        // 系统图反序列化
        if (j.contains("systemDiagrams") && j["systemDiagrams"].is_array()) {
            for (const auto& sdj : j["systemDiagrams"]) {
                SystemDiagram sd;
                sd.diagramId = sdj.value("diagramId", "");
                sd.floorId = sdj.value("floorId", "");
                if (sdj.contains("nodes") && sdj["nodes"].is_array()) {
                    for (const auto& nj : sdj["nodes"]) {
                        SystemNode n;
                        n.nodeId = nj.value("nodeId", "");
                        n.type = (NodeType)nj.value("type", 0);
                        n.deviceInstanceId = nj.value("deviceInstanceId", "");
                        n.layoutPos.x = nj.value("layoutX", 0.0);
                        n.layoutPos.y = nj.value("layoutY", 0.0);
                        n.label = nj.value("label", "");
                        sd.nodes.push_back(n);
                    }
                }
                if (sdj.contains("links") && sdj["links"].is_array()) {
                    for (const auto& lj : sdj["links"]) {
                        SystemLink l;
                        l.linkId = lj.value("linkId", "");
                        l.fromNodeId = lj.value("fromNodeId", "");
                        l.toNodeId = lj.value("toNodeId", "");
                        l.cableModelId = lj.value("cableModelId", "");
                        l.length_m = lj.value("length_m", 0.0);
                        l.loss_dB = lj.value("loss_dB", 0.0);
                        sd.links.push_back(l);
                    }
                }
                p->systemDiagrams.push_back(sd);
            }
        }
        if (j.contains("globalAuditLog"))
            deserializeAudit(j["globalAuditLog"], p->globalAuditLog);
    }

    void serializeFloor(const Floor& f, json& j) {
        j["floorId"] = f.floorId;
        j["floorName"] = f.floorName;
        j["floorIndex"] = f.floorIndex;
        j["elevation_m"] = f.elevation_m;
        j["height_m"] = f.height_m;
        j["netHeight_m"] = f.netHeight_m;
        j["isStandardFloor"] = f.isStandardFloor;
        j["origin_x"] = f.origin.x;
        j["origin_y"] = f.origin.y;
        j["drawingScale"] = f.drawingScale;
        // 墙体
        json walls = json::array();
        for (const auto& w : f.walls) {
            json wj;
            wj["wallId"] = w.wallId;
            wj["material"] = static_cast<int>(w.material);
            wj["thickness_mm"] = w.thickness_mm;
            wj["height_m"] = w.height_m;
            wj["attenuation_dB"] = w.attenuation_dB;
            json pts = json::array();
            for (const auto& pt : w.points) {
                pts.push_back({pt.x, pt.y});
            }
            wj["points"] = pts;
            walls.push_back(wj);
        }
        j["walls"] = walls;
        // 馈线段
        json cables = json::array();
        for (const auto& c : f.cables) {
            json cj;
            cj["segmentId"] = c.segmentId;
            cj["modelId"] = c.modelId;
            cj["floorId"] = c.floorId;
            cj["length_m"] = c.length_m;
            cj["fromDeviceId"] = c.fromDeviceId;
            cj["toDeviceId"] = c.toDeviceId;
            json pts = json::array();
            for (const auto& pt : c.routePoints) {
                pts.push_back({pt.x, pt.y});
            }
            cj["routePoints"] = pts;
            cables.push_back(cj);
        }
        j["cables"] = cables;
        json devs = json::array();
        for (const auto& d : f.devices) {
            json dj;
            serializeDevice(d, dj);
            devs.push_back(dj);
        }
        j["devices"] = devs;
        serializeAudit(f.floorAuditLog, j["floorAuditLog"]);
    }

    void deserializeFloor(const json& j, Floor& f) {
        f.floorId = j.value("floorId", "");
        f.floorName = j.value("floorName", "");
        f.floorIndex = j.value("floorIndex", 0);
        f.elevation_m = j.value("elevation_m", 0.0);
        f.height_m = j.value("height_m", 3.0);
        f.netHeight_m = j.value("netHeight_m", 2.8);
        f.isStandardFloor = j.value("isStandardFloor", false);
        f.origin.x = j.value("origin_x", 0.0);
        f.origin.y = j.value("origin_y", 0.0);
        f.drawingScale = j.value("drawingScale", 100.0);
        // 墙体
        if (j.contains("walls") && j["walls"].is_array()) {
            for (const auto& wj : j["walls"]) {
                Wall w;
                w.wallId = wj.value("wallId", "");
                w.material = static_cast<WallMaterial>(wj.value("material", 0));
                w.thickness_mm = wj.value("thickness_mm", 200.0);
                w.height_m = wj.value("height_m", 3.0);
                w.attenuation_dB = wj.value("attenuation_dB", 0.0);
                if (wj.contains("points") && wj["points"].is_array()) {
                    for (const auto& pt : wj["points"]) {
                        Point2D p;
                        p.x = pt[0].get<double>();
                        p.y = pt[1].get<double>();
                        w.points.push_back(p);
                    }
                }
                f.walls.push_back(w);
            }
        }
        // 馈线段
        if (j.contains("cables") && j["cables"].is_array()) {
            for (const auto& cj : j["cables"]) {
                CableSegment c;
                c.segmentId = cj.value("segmentId", "");
                c.modelId = cj.value("modelId", "");
                c.floorId = cj.value("floorId", "");
                c.length_m = cj.value("length_m", 0.0);
                c.fromDeviceId = cj.value("fromDeviceId", "");
                c.toDeviceId = cj.value("toDeviceId", "");
                if (cj.contains("routePoints") && cj["routePoints"].is_array()) {
                    for (const auto& pt : cj["routePoints"]) {
                        Point2D p;
                        p.x = pt[0].get<double>();
                        p.y = pt[1].get<double>();
                        c.routePoints.push_back(p);
                    }
                }
                f.cables.push_back(c);
            }
        }
        if (j.contains("devices") && j["devices"].is_array()) {
            for (const auto& dj : j["devices"]) {
                DeviceInstance d;
                deserializeDevice(dj, d);
                f.devices.push_back(std::move(d));
            }
        }
        if (j.contains("floorAuditLog"))
            deserializeAudit(j["floorAuditLog"], f.floorAuditLog);
    }

    void serializeDevice(const DeviceInstance& d, json& j) {
        j["instanceId"] = d.instanceId;
        j["modelId"] = d.modelId;
        j["label"] = d.label;
        j["userNote"] = d.userNote;
        j["position_x"] = d.position.x;
        j["position_y"] = d.position.y;
        j["rotation"] = d.rotation;
        j["elevation_m"] = d.elevation_m;
        j["floorId"] = d.floorId;
        j["locked"] = d.locked;
        j["visible"] = d.visible;
        j["sketchPlaceholder"] = d.sketchPlaceholder;
        j["status"] = static_cast<int>(d.status);
    }

    void deserializeDevice(const json& j, DeviceInstance& d) {
        d.instanceId = j.value("instanceId", "");
        d.modelId = j.value("modelId", "");
        d.label = j.value("label", "");
        d.userNote = j.value("userNote", "");
        d.position.x = j.value("position_x", 0.0);
        d.position.y = j.value("position_y", 0.0);
        d.rotation = j.value("rotation", 0.0);
        d.elevation_m = j.value("elevation_m", 0.0);
        d.floorId = j.value("floorId", "");
        d.locked = j.value("locked", false);
        d.visible = j.value("visible", true);
        d.sketchPlaceholder = j.value("sketchPlaceholder", false);
        d.status = static_cast<LinkStatus>(j.value("status", 0));
    }

    void serializeAudit(const std::vector<AuditEntry>& log, json& j) {
        json arr = json::array();
        for (const auto& e : log) {
            json ej;
            ej["entryId"] = e.entryId;
            ej["actionType"] = e.actionType;
            ej["operatorInfo"] = e.operatorInfo;
            ej["comment"] = e.comment;
            ej["isHighRisk"] = e.isHighRisk;
            ej["targetObjectIds"] = e.targetObjectIds;
            arr.push_back(ej);
        }
        j = arr;
    }

    void deserializeAudit(const json& j, std::vector<AuditEntry>& log) {
        if (!j.is_array()) return;
        for (const auto& ej : j) {
            AuditEntry e;
            e.entryId = ej.value("entryId", "");
            e.actionType = ej.value("actionType", "");
            e.operatorInfo = ej.value("operatorInfo", "");
            e.comment = ej.value("comment", "");
            e.isHighRisk = ej.value("isHighRisk", false);
            if (ej.contains("targetObjectIds") && ej["targetObjectIds"].is_array()) {
                for (const auto& id : ej["targetObjectIds"])
                    e.targetObjectIds.push_back(id.get<std::string>());
            }
            log.push_back(std::move(e));
        }
    }
};

} // namespace zf
