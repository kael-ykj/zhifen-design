#pragma once

#include <string>
#include "zf_types.h"
#include "zf_error.h"
#include "../mode_control/mode_control_layer.h"

namespace zf {

enum class DwgExportMode {
    SKETCH_EXPORT = 0,
    FORMAL_EXPORT = 1
};

class DwgExporter {
public:
    DwgExporter() = default;

    void setModeManager(ModeManager* mgr) { m_modeMgr = mgr; }

    int exportDwg(const std::string& filePath, const Project* project, DwgExportMode mode) {
        if (!project) return ZF_ERR_ARG;
        if (filePath.empty()) return ZF_ERR_ARG_EMPTY;

        if (mode == DwgExportMode::FORMAL_EXPORT && m_modeMgr) {
            int perm = m_modeMgr->checkHeavyComputePermission();
            if (perm != ZF_ERR_OK) return perm;
        }

        if (mode == DwgExportMode::SKETCH_EXPORT)
            return doSketchExport(filePath, project);
        else
            return doFormalExport(filePath, project);
    }

    int exportPdf(const std::string& filePath, const Project* project, DwgExportMode mode) {
        if (!project) return ZF_ERR_ARG;
        if (mode == DwgExportMode::FORMAL_EXPORT && m_modeMgr) {
            int perm = m_modeMgr->checkHeavyComputePermission();
            if (perm != ZF_ERR_OK) return perm;
        }
        return ZF_ERR_OK;
    }

private:
    ModeManager* m_modeMgr{nullptr};

    int doSketchExport(const std::string&, const Project*) {
        // 草图导出：剥离业务XData，跳过校验，纯几何输出
        // TODO: 接入ODA/Teigha底层DWG写入
        return ZF_ERR_OK;
    }

    int doFormalExport(const std::string&, const Project* project) {
        int check = runFormalChecks(project);
        if (check != ZF_ERR_OK) return check;
        // TODO: 底层DWG写入 + XData附加
        return ZF_ERR_OK;
    }

    int runFormalChecks(const Project* project) {
        if (!project) return ZF_ERR_ARG;
        return ZF_ERR_OK;
    }
};

} // namespace zf
