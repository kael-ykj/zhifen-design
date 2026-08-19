#pragma once

namespace zf {

constexpr int ZF_ERR_OK                      = 0;
constexpr int ZF_ERR_UNKNOWN                 = -1;
constexpr int ZF_ERR_NOT_IMPLEMENTED         = -2;
constexpr int ZF_ERR_NULLPTR                 = -3;

// 模式与权限限制 -1000段
constexpr int ZF_ERR_SKETCH_MODE_RESTRICTED  = -1001;
constexpr int ZF_ERR_FORMAL_MODE_REQUIRED    = -1002;
constexpr int ZF_ERR_PERMISSION_DENIED       = -1003;

// IO与文件 -2000段
constexpr int ZF_ERR_IO_OPEN_FAILED          = -2001;
constexpr int ZF_ERR_IO_READ_FAILED          = -2002;
constexpr int ZF_ERR_IO_WRITE_FAILED         = -2003;
constexpr int ZF_ERR_FILE_FORMAT             = -2004;
constexpr int ZF_ERR_VERSION_TOO_OLD         = -2005;
constexpr int ZF_ERR_VERSION_TOO_NEW         = -2006;

// 参数与数据校验 -3000段
constexpr int ZF_ERR_ARG_INVALID             = -3001;
constexpr int ZF_ERR_ARG_EMPTY               = -3002;
constexpr int ZF_ERR_NOT_FOUND               = -3003;
constexpr int ZF_ERR_DUPLICATE_ID            = -3004;
constexpr int ZF_ERR_DEVICE_MODEL_NOT_FOUND  = -3005;
constexpr int ZF_ERR_TOPOLOGY_BROKEN         = -3006;

// 几何与图形 -4000段
constexpr int ZF_ERR_GEOMETRY_INVALID        = -4001;
constexpr int ZF_ERR_GEOMETRY_DEGENERATE     = -4002;

// 链路计算 -5000段
constexpr int ZF_ERR_LINK_NO_SOURCE          = -5001;
constexpr int ZF_ERR_LINK_LOOP               = -5002;
constexpr int ZF_ERR_LINK_OVER_LOSS          = -5003;
constexpr int ZF_ERR_LINK_POWER_OVERLIMIT    = -5004;

// 仿真计算 -6000段
constexpr int ZF_ERR_SIM_NO_ANTENNA          = -6001;
constexpr int ZF_ERR_SIM_GRID_TOO_DENSE      = -6002;
constexpr int ZF_ERR_SIM_OUT_OF_MEMORY       = -6003;

// 导出与报表 -7000段
constexpr int ZF_ERR_EXPORT_EMPTY            = -7001;
constexpr int ZF_ERR_EXPORT_TEMPLATE         = -7002;

// AI引擎 -8000段
constexpr int ZF_ERR_AI_MODEL_NOT_LOADED     = -8001;
constexpr int ZF_ERR_AI_HALLUCINATION_RISK   = -8002;

inline const char* zfErrorString(int code)
{
    switch (code) {
        case ZF_ERR_OK:                      return "操作成功";
        case ZF_ERR_SKETCH_MODE_RESTRICTED:  return "草图模式限制：请切换至正式工程模式后执行此操作";
        case ZF_ERR_FORMAL_MODE_REQUIRED:    return "此功能仅正式工程模式可用";
        case ZF_ERR_IO_OPEN_FAILED:          return "文件打开失败";
        case ZF_ERR_ARG_INVALID:             return "参数无效";
        case ZF_ERR_NOT_FOUND:               return "对象不存在";
        default:                             return "未知错误";
    }
}

} // namespace zf
