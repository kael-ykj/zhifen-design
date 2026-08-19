#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include "zf_types.h"
#include "zf_error.h"

namespace zf {

class CheckSwitchController {
public:
    void syncByWorkMode(WorkMode mode) {
        if (mode == WorkMode::SKETCH_MODE) {
            enableLinkAutoRecalc = false;
            enableAutoTopoRepair = false;
            enableParamCompleteCheck = false;
            enableBomRealTimeStat = false;
            enablePortSemanticSnap = false;
        } else {
            enableLinkAutoRecalc = true;
            enableAutoTopoRepair = true;
            enableParamCompleteCheck = true;
            enableBomRealTimeStat = true;
            enablePortSemanticSnap = true;
        }
    }

    bool enableLinkAutoRecalc{false};
    bool enableAutoTopoRepair{false};
    bool enableParamCompleteCheck{false};
    bool enableBomRealTimeStat{false};
    bool enablePortSemanticSnap{false};
};

class InteractionConstraintInterceptor {
public:
    void syncByWorkMode(WorkMode mode) {
        if (mode == WorkMode::SKETCH_MODE) {
            m_blockUi = false;
            m_interruptEdit = false;
        } else {
            m_blockUi = true;
            m_interruptEdit = true;
        }
    }
    bool allowBlockUiAlert() const { return m_blockUi; }
    bool allowInterruptEditSaveExport() const { return m_interruptEdit; }
private:
    bool m_blockUi{false};
    bool m_interruptEdit{false};
};

class ModeManager {
public:
    ModeManager() {
        m_switchCtrl.syncByWorkMode(m_workMode);
        m_interceptor.syncByWorkMode(m_workMode);
    }

    WorkMode getGlobalWorkMode() const { return m_workMode; }

    void setGlobalWorkMode(WorkMode mode, const std::string& opUser = "user") {
        if (mode == m_workMode) return;
        WorkMode oldMode = m_workMode;
        m_workMode = mode;
        m_switchCtrl.syncByWorkMode(mode);
        m_interceptor.syncByWorkMode(mode);

        AuditEntry entry;
        entry.entryId = generateUuid();
        entry.timestamp = std::chrono::system_clock::now();
        entry.operatorInfo = opUser;
        entry.actionType = "mode_switch";
        entry.comment = std::string("从") + (oldMode == WorkMode::SKETCH_MODE ? "草图模式" : "正式模式")
                      + "切换为" + (mode == WorkMode::SKETCH_MODE ? "草图模式" : "正式模式");
        entry.isHighRisk = false;
        appendAuditEntry(entry);
    }

    int checkHeavyComputePermission() const {
        if (m_workMode == WorkMode::SKETCH_MODE)
            return ZF_ERR_SKETCH_MODE_RESTRICTED;
        return ZF_ERR_OK;
    }

    void appendAuditEntry(const AuditEntry& entry) {
        m_auditLog.push_back(entry);
        if (m_boundProject)
            m_boundProject->globalAuditLog.push_back(entry);
    }

    const std::vector<AuditEntry>& getAuditLog() const { return m_auditLog; }
    void clearAuditLog() { m_auditLog.clear(); }

    void bindProject(Project* project) { m_boundProject = project; }
    void unbindProject() { m_boundProject = nullptr; }

    int restoreFromProject() {
        if (!m_boundProject) return ZF_ERR_ARG;
        m_workMode = m_boundProject->globalWorkMode;
        m_switchCtrl.syncByWorkMode(m_workMode);
        m_interceptor.syncByWorkMode(m_workMode);
        m_auditLog = m_boundProject->globalAuditLog;
        return ZF_ERR_OK;
    }

    CheckSwitchController* switchController() { return &m_switchCtrl; }
    InteractionConstraintInterceptor* interceptor() { return &m_interceptor; }

private:
    WorkMode m_workMode{WorkMode::SKETCH_MODE};
    Project* m_boundProject{nullptr};
    std::vector<AuditEntry> m_auditLog;
    CheckSwitchController m_switchCtrl;
    InteractionConstraintInterceptor m_interceptor;

    static std::string generateUuid() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        ss << std::setw(8) << dis(gen) << "-"
           << std::setw(4) << (dis(gen) & 0xFFFF) << "-"
           << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << "-"
           << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << "-"
           << std::setw(12) << dis(gen);
        return ss.str();
    }
};

class ModeControlLayer {
public:
    ModeControlLayer() { modeManager = std::make_unique<ModeManager>(); }
    ~ModeControlLayer() = default;
    int init() { return ZF_ERR_OK; }
    void release() { modeManager.reset(); }
    std::unique_ptr<ModeManager> modeManager;
};

} // namespace zf
