#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "zf_error.h"
#include "zf_types.h"

namespace zf {

struct AlertItem {
    std::string alertId;
    std::string targetItemId;
    int errorCode{ZF_ERR_OK};
    std::string message;
    CheckSeverity severity{CheckSeverity::INFO};
    bool markedOnCanvas{true};
};

class AlertMarker {
public:
    AlertMarker() = default;

    void addAlert(const std::string& itemId, int code, const std::string& msg, CheckSeverity sev) {
        AlertItem item;
        item.alertId = "ALT_" + std::to_string(m_alerts.size());
        item.targetItemId = itemId;
        item.errorCode = code;
        item.message = msg.empty() ? zfErrorString(code) : msg;
        item.severity = sev;
        item.markedOnCanvas = true;
        m_alerts.push_back(std::move(item));
    }

    void clearAlertsByItem(const std::string& itemId) {
        auto it = std::remove_if(m_alerts.begin(), m_alerts.end(),
            [&](const AlertItem& a) { return a.targetItemId == itemId; });
        if (it != m_alerts.end()) m_alerts.erase(it, m_alerts.end());
    }

    void clearAll() { m_alerts.clear(); }

    std::vector<AlertItem> allAlerts() const { return m_alerts; }

    bool hasAlert(const std::string& itemId) const {
        for (const auto& a : m_alerts)
            if (a.targetItemId == itemId) return true;
        return false;
    }

    void syncByWorkMode(WorkMode mode) {
        m_currentMode = mode;
        m_blockUiDialog = (mode == WorkMode::FORMAL_MODE);
    }

    bool blockUiDialog() const { return m_blockUiDialog; }
    int alertCount() const { return static_cast<int>(m_alerts.size()); }

private:
    std::vector<AlertItem> m_alerts;
    WorkMode m_currentMode{WorkMode::SKETCH_MODE};
    bool m_blockUiDialog{false};
};

} // namespace zf
