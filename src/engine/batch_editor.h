#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "device/device_library.h"

namespace zf {

// CSV 行数据
struct CsvRow {
    std::map<std::string, std::string> fields;
    std::string get(const std::string& key, const std::string& def = "") const {
        auto it = fields.find(key);
        return it != fields.end() ? it->second : def;
    }
    bool has(const std::string& key) const {
        return fields.count(key) > 0;
    }
};

// 批量导入结果
struct BatchImportResult {
    int totalRows{0};
    int successCount{0};
    int failedCount{0};
    std::vector<std::string> errors;
    std::vector<DeviceInstance> importedDevices;
};

// 批量编辑器
class BatchEditor {
public:
    BatchEditor() = default;

    // 解析CSV文件
    int parseCsv(const std::string& filePath, std::vector<CsvRow>& outRows) {
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) return ZF_ERR_IO;

        outRows.clear();
        std::string line;

        // 读取表头
        if (!std::getline(ifs, line)) return ZF_ERR_IO;
        std::vector<std::string> headers = splitCsvLine(line);

        // 读取数据行
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            std::vector<std::string> values = splitCsvLine(line);
            CsvRow row;
            for (size_t i = 0; i < headers.size() && i < values.size(); i++) {
                row.fields[headers[i]] = values[i];
            }
            outRows.push_back(row);
        }
        ifs.close();
        return ZF_ERR_OK;
    }

    // 从CSV批量导入器件
    int importDevicesFromCsv(const std::string& filePath,
                             const DeviceLibrary* devLib,
                             Floor* floor,
                             BatchImportResult& outResult) {
        if (!devLib || !floor) return ZF_ERR_ARG;

        std::vector<CsvRow> rows;
        int parseResult = parseCsv(filePath, rows);
        if (parseResult != ZF_ERR_OK) return parseResult;

        outResult.totalRows = rows.size();
        outResult.successCount = 0;
        outResult.failedCount = 0;
        outResult.errors.clear();
        outResult.importedDevices.clear();

        for (size_t i = 0; i < rows.size(); i++) {
            const auto& row = rows[i];
            std::string modelId = row.get("modelId", row.get("型号", ""));
            std::string instanceId = row.get("instanceId", row.get("编号", ""));
            double x = std::stod(row.get("x", row.get("X", "0")));
            double y = std::stod(row.get("y", row.get("Y", "0")));

            // 校验型号
            if (!devLib->getModelById(modelId)) {
                outResult.failedCount++;
                outResult.errors.push_back("第" + std::to_string(i+2) + "行: 未知型号 '" + modelId + "'");
                continue;
            }

            DeviceInstance dev;
            dev.instanceId = instanceId.empty() ? ("DEV_" + std::to_string(floor->devices.size() + i)) : instanceId;
            dev.modelId = modelId;
            dev.position = {x, y};
            dev.userNote = row.get("note", row.get("备注", ""));

            floor->devices.push_back(dev);
            outResult.importedDevices.push_back(dev);
            outResult.successCount++;
        }

        return ZF_ERR_OK;
    }

    // 批量修改器件属性
    int batchUpdateProperty(Floor* floor,
                            const std::vector<std::string>& instanceIds,
                            const std::string& property,
                            const std::string& value) {
        if (!floor) return ZF_ERR_ARG;
        int updated = 0;

        for (auto& dev : floor->devices) {
            if (std::find(instanceIds.begin(), instanceIds.end(), dev.instanceId) != instanceIds.end()) {
                if (property == "userNote" || property == "备注") {
                    dev.userNote = value;
                    updated++;
                } else if (property == "modelId" || property == "型号") {
                    dev.modelId = value;
                    updated++;
                }
                // 其他属性可扩展
            }
        }
        return updated;
    }

    // 批量删除器件
    int batchDeleteDevices(Floor* floor, const std::vector<std::string>& instanceIds) {
        if (!floor) return ZF_ERR_ARG;
        int before = floor->devices.size();
        floor->devices.erase(
            std::remove_if(floor->devices.begin(), floor->devices.end(),
                [&](const DeviceInstance& d) {
                    return std::find(instanceIds.begin(), instanceIds.end(), d.instanceId) != instanceIds.end();
                }),
            floor->devices.end()
        );
        return before - floor->devices.size();
    }

    // 批量移动器件
    int batchMoveDevices(Floor* floor,
                         const std::vector<std::string>& instanceIds,
                         double dx, double dy) {
        if (!floor) return ZF_ERR_ARG;
        int moved = 0;
        for (auto& dev : floor->devices) {
            if (std::find(instanceIds.begin(), instanceIds.end(), dev.instanceId) != instanceIds.end()) {
                dev.position.x += dx;
                dev.position.y += dy;
                moved++;
            }
        }
        return moved;
    }

    // 导出器件清单到CSV
    int exportDevicesToCsv(const std::string& filePath, const Floor* floor) {
        if (!floor) return ZF_ERR_ARG;

        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return ZF_ERR_IO;

        ofs << "instanceId,modelId,x,y,userNote\n";
        for (const auto& dev : floor->devices) {
            ofs << dev.instanceId << ","
                << dev.modelId << ","
                << dev.position.x << ","
                << dev.position.y << ","
                << dev.userNote << "\n";
        }
        ofs.close();
        return ZF_ERR_OK;
    }

    // 按型号统计数量
    std::map<std::string, int> countByModel(const Floor* floor) const {
        std::map<std::string, int> counts;
        if (!floor) return counts;
        for (const auto& dev : floor->devices) {
            counts[dev.modelId]++;
        }
        return counts;
    }

private:
    // 简单CSV行分割（支持引号）
    std::vector<std::string> splitCsvLine(const std::string& line) {
        std::vector<std::string> result;
        std::string current;
        bool inQuotes = false;

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                result.push_back(trim(current));
                current.clear();
            } else {
                current += c;
            }
        }
        result.push_back(trim(current));
        return result;
    }

    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};

} // namespace zf
