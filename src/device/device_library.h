#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {

class DeviceLibrary {
public:
    DeviceLibrary() = default;

    int loadDefaultLibrary() {
        m_models.clear();
        initBuiltinAntennas();
        initBuiltinSplitters();
        initBuiltinCouplers();
        initBuiltinCombiners();
        initBuiltinSources();
        initBuiltinCables();
        return ZF_ERR_OK;
    }

    std::optional<DeviceModel> getModelById(const std::string& modelId) const {
        auto it = m_models.find(modelId);
        if (it == m_models.end()) return std::nullopt;
        return it->second;
    }

    std::vector<DeviceModel> getModelsByCategory(DeviceCategory category) const {
        std::vector<DeviceModel> result;
        for (const auto& pair : m_models) {
            if (pair.second.category == category)
                result.push_back(pair.second);
        }
        return result;
    }

    int addOrUpdateModel(const DeviceModel& model) {
        m_models[model.modelId] = model;
        return ZF_ERR_OK;
    }

    size_t modelCount() const { return m_models.size(); }

private:
    std::unordered_map<std::string, DeviceModel> m_models;

    void initBuiltinAntennas() {
        DeviceModel ant;
        ant.modelId = "ANT_OMNI_CEILING";
        ant.displayName = "全向吸顶天线";
        ant.category = DeviceCategory::ANTENNA;
        ant.freqMinMHz = 800;
        ant.freqMaxMHz = 2700;
        ant.gain_dBi = 3.0;
        ant.vswr = 1.3;
        ant.powerCapacity_W = 100;
        ant.symbolName = "antenna_omni";
        m_models[ant.modelId] = ant;

        DeviceModel wall;
        wall.modelId = "ANT_WALL_MOUNT";
        wall.displayName = "壁挂定向天线";
        wall.category = DeviceCategory::ANTENNA;
        wall.gain_dBi = 7.0;
        wall.symbolName = "antenna_wall";
        m_models[wall.modelId] = wall;
    }

    void initBuiltinSplitters() {
        DeviceModel sp2;
        sp2.modelId = "SPLIT_2WAY";
        sp2.displayName = "二功分器";
        sp2.category = DeviceCategory::SPLITTER;
        sp2.insertionLoss_dB = 3.5;
        sp2.isolation_dB = 25;
        sp2.portCount = 3;
        sp2.symbolName = "splitter_2";
        m_models[sp2.modelId] = sp2;

        DeviceModel sp3;
        sp3.modelId = "SPLIT_3WAY";
        sp3.displayName = "三功分器";
        sp3.category = DeviceCategory::SPLITTER;
        sp3.insertionLoss_dB = 5.5;
        sp3.symbolName = "splitter_3";
        m_models[sp3.modelId] = sp3;

        DeviceModel sp4;
        sp4.modelId = "SPLIT_4WAY";
        sp4.displayName = "四功分器";
        sp4.category = DeviceCategory::SPLITTER;
        sp4.insertionLoss_dB = 6.5;
        sp4.symbolName = "splitter_4";
        m_models[sp4.modelId] = sp4;
    }

    void initBuiltinCouplers() {
        std::vector<int> dbs = {5, 6, 7, 10, 15, 20};
        for (int db : dbs) {
            DeviceModel cp;
            cp.modelId = "COUP_" + std::to_string(db) + "DB";
            cp.displayName = std::to_string(db) + "dB耦合器";
            cp.category = DeviceCategory::COUPLER;
            cp.couplingLoss_dB = db;
            cp.throughLoss_dB = 0.2 + 0.03 * db;
            cp.symbolName = "coupler_right";
            m_models[cp.modelId] = cp;
        }
    }

    void initBuiltinCombiners() {
        DeviceModel cb2;
        cb2.modelId = "COMB_2IN";
        cb2.displayName = "二合一合路器";
        cb2.category = DeviceCategory::COMBINER;
        cb2.insertionLoss_dB = 0.8;
        cb2.isolation_dB = 30;
        cb2.symbolName = "combiner_2";
        m_models[cb2.modelId] = cb2;
    }

    void initBuiltinSources() {
        DeviceModel rru;
        rru.modelId = "SRC_RRU";
        rru.displayName = "RRU信源";
        rru.category = DeviceCategory::SIGNAL_SOURCE;
        rru.symbolName = "source_rru";
        m_models[rru.modelId] = rru;

        DeviceModel cp;
        cp.modelId = "SRC_CONNECT_POINT";
        cp.displayName = "P连接点";
        cp.category = DeviceCategory::SIGNAL_SOURCE;
        cp.symbolName = "connect_point";
        m_models[cp.modelId] = cp;
    }

    void initBuiltinCables() {
        DeviceModel cab12;
        cab12.modelId = "CABLE_1_2";
        cab12.displayName = "1/2馈线";
        cab12.category = DeviceCategory::CABLE;
        cab12.lossPer100m_900MHz = 6.2;
        cab12.lossPer100m_2100MHz = 10.5;
        cab12.lineWidth = 1.0;
        m_models[cab12.modelId] = cab12;

        DeviceModel cab78;
        cab78.modelId = "CABLE_7_8";
        cab78.displayName = "7/8馈线";
        cab78.category = DeviceCategory::CABLE;
        cab78.lossPer100m_900MHz = 3.9;
        cab78.lossPer100m_2100MHz = 6.5;
        cab78.lineWidth = 2.0;
        m_models[cab78.modelId] = cab78;
    }
};

} // namespace zf
