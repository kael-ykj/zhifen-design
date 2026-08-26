#pragma once
#include "core/zf_types.h"
#include <cmath>
#include <string>
#include <vector>

namespace zf {

// 自动布放参数
struct AutoPlaceParams {
    double coverageRadius_m = 8.0;    // 天线覆盖半径（米）
    double sourcePower_dBm = 43.0;    // 信源输出功率
    std::string antennaModel = "ANT_OMNI_5dBi";
    std::string sourceModel = "SRC_BBU_RRU";
    std::string splitter2Model = "SPLIT_2WAY";
    std::string splitter4Model = "SPLIT_4WAY";
};

// 自动布放结果
struct AutoPlaceResult {
    bool success = false;
    int antennaCount = 0;
    int splitterCount = 0;
    int sourceCount = 0;
    int connectionCount = 0;
    std::string message;
};

// 自动布放引擎
class AutoPlacer {
public:
    AutoPlacer() = default;

    // 执行自动布放，直接修改floor对象
    AutoPlaceResult place(Floor& floor, const AutoPlaceParams& params = AutoPlaceParams()) {
        AutoPlaceResult result;

        // 1. 计算楼层边界
        double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
        bool hasBounds = false;
        for (const auto& wall : floor.walls) {
            for (const auto& pt : wall.points) {
                minX = std::min(minX, pt.x);
                minY = std::min(minY, pt.y);
                maxX = std::max(maxX, pt.x);
                maxY = std::max(maxY, pt.y);
                hasBounds = true;
            }
        }
        if (!hasBounds) {
            // 没有墙体，使用默认尺寸 800x600
            minX = 0; minY = 0; maxX = 800; maxY = 600;
        }

        double width = maxX - minX;
        double height = maxY - minY;
        if (width < 10 || height < 10) {
            result.message = "楼层尺寸过小，无法自动布放";
            return result;
        }

        // 2. 计算网格间距（覆盖半径的1.5倍，保证重叠覆盖）
        double spacing = params.coverageRadius_m * 1.5;
        int cols = std::max(1, (int)std::ceil(width / spacing));
        int rows = std::max(1, (int)std::ceil(height / spacing));

        // 3. 清除原有器件和连接（保留墙体）
        floor.devices.clear();

        // 4. 放置信源（在左上角）
        DeviceInstance source;
        source.instanceId = "SRC_AUTO_1";
        source.modelId = params.sourceModel;
        source.position = {minX + 30, minY + 30};
        source.label = "信源";
        floor.devices.push_back(source);
        result.sourceCount = 1;

        // 5. 计算天线位置（网格布局，居中）
        double startX = minX + (width - (cols - 1) * spacing) / 2;
        double startY = minY + (height - (rows - 1) * spacing) / 2;

        std::vector<std::string> antennaIds;
        int antIdx = 0;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                DeviceInstance ant;
                ant.instanceId = "ANT_AUTO_" + std::to_string(++antIdx);
                ant.modelId = params.antennaModel;
                ant.position = {startX + c * spacing, startY + r * spacing};
                ant.label = "天线" + std::to_string(antIdx);
                floor.devices.push_back(ant);
                antennaIds.push_back(ant.instanceId);
            }
        }
        result.antennaCount = antIdx;

        // 6. 放置功分器（4功分为主，剩余用2功分）
        int remaining = antIdx;
        int splitter4Count = remaining / 4;
        int splitter2Count = (remaining % 4 >= 2) ? 1 : 0;
        int directCount = remaining % 2; // 剩余1个直接连

        std::vector<std::string> splitterIds;
        int splIdx = 0;

        // 功分器位置：在信源右侧排列
        double splX = minX + 120;
        double splY = minY + 30;
        double splSpacing = 60;

        for (int i = 0; i < splitter4Count; ++i) {
            DeviceInstance spl;
            spl.instanceId = "SPL_AUTO_" + std::to_string(++splIdx);
            spl.modelId = params.splitter4Model;
            spl.position = {splX, splY + i * splSpacing};
            spl.label = "4功分" + std::to_string(i + 1);
            floor.devices.push_back(spl);
            splitterIds.push_back(spl.instanceId);
        }
        if (splitter2Count) {
            DeviceInstance spl;
            spl.instanceId = "SPL_AUTO_" + std::to_string(++splIdx);
            spl.modelId = params.splitter2Model;
            spl.position = {splX, splY + splitter4Count * splSpacing};
            spl.label = "2功分";
            floor.devices.push_back(spl);
            splitterIds.push_back(spl.instanceId);
        }
        result.splitterCount = splIdx;

        // 7. 连接线缆
        // 信源 -> 功分器（如果有多个功分器，信源先连第一个，然后级联）
        auto& srcDev = floor.devices[0];
        if (!splitterIds.empty()) {
            // 信源连第一个功分器
            DeviceInstance::Connection conn;
            conn.targetInstanceId = splitterIds[0];
            conn.fromPortId = "out";
            conn.toPortId = "in";
            srcDev.connections.push_back(conn);
            result.connectionCount++;

            // 功分器级联（第一个的一个输出口连下一个的输入）
            for (size_t i = 0; i + 1 < splitterIds.size(); ++i) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == splitterIds[i]) {
                        DeviceInstance::Connection c;
                        c.targetInstanceId = splitterIds[i + 1];
                        c.fromPortId = "out1";
                        c.toPortId = "in";
                        dev.connections.push_back(c);
                        result.connectionCount++;
                        break;
                    }
                }
            }
        }

        // 功分器 -> 天线
        int antAssign = 0;
        for (size_t si = 0; si < splitterIds.size(); ++si) {
            bool is4Way = (si < (size_t)splitter4Count);
            int outputs = is4Way ? 4 : 2;
            for (int p = 0; p < outputs && antAssign < (int)antennaIds.size(); ++p) {
                for (auto& dev : floor.devices) {
                    if (dev.instanceId == splitterIds[si]) {
                        DeviceInstance::Connection c;
                        c.targetInstanceId = antennaIds[antAssign];
                        c.fromPortId = "out" + std::to_string(p + 1);
                        c.toPortId = "in";
                        dev.connections.push_back(c);
                        result.connectionCount++;
                        break;
                    }
                }
                antAssign++;
            }
        }

        // 如果还有剩余天线（directCount），直接从信源或最后一个功分器连
        while (antAssign < (int)antennaIds.size()) {
            // 从最后一个功分器或信源连
            std::string fromId = splitterIds.empty() ? source.instanceId : splitterIds.back();
            for (auto& dev : floor.devices) {
                if (dev.instanceId == fromId) {
                    DeviceInstance::Connection c;
                    c.targetInstanceId = antennaIds[antAssign];
                    c.fromPortId = "out";
                    c.toPortId = "in";
                    dev.connections.push_back(c);
                    result.connectionCount++;
                    break;
                }
            }
            antAssign++;
        }

        result.success = true;
        result.message = "自动布放完成: " + std::to_string(antIdx) + "个天线, " +
                         std::to_string(splIdx) + "个功分器, " +
                         std::to_string(result.connectionCount) + "条连接";
        return result;
    }
};

} // namespace zf
