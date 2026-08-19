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
