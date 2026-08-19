#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "core/zf_types.h"
#include "core/zf_error.h"

namespace zf {

// 复制选项
struct CloneOptions {
    bool cloneWalls{true};
    bool cloneDoors{true};
    bool cloneWindows{true};
    bool cloneDevices{true};
    bool cloneCables{true};
    bool cloneRooms{true};
    bool cloneBackground{false};  // 底图不复制（各层独立）
    double offsetX{0.0};
    double offsetY{0.0};
    std::string idPrefix{""};     // ID前缀，避免冲突
};

// 标准层复制引擎
class FloorCloner {
public:
    FloorCloner() = default;

    // 标记楼层为标准层
    int markAsStandard(Floor* floor, const std::string& standardName = "") {
        if (!floor) return ZF_ERR_ARG;
        floor->isStandardFloor = true;
        if (!standardName.empty()) {
            floor->metadata["standardName"] = standardName;
        }
        return ZF_ERR_OK;
    }

    // 取消标准层标记
    int unmarkStandard(Floor* floor) {
        if (!floor) return ZF_ERR_ARG;
        floor->isStandardFloor = false;
        floor->referenceFloorId.clear();
        return ZF_ERR_OK;
    }

    // 从标准层复制到新楼层
    int cloneFromStandard(const Floor* source, const std::string& newFloorId,
                          const std::string& newFloorName,
                          const CloneOptions& options,
                          Floor& outFloor) {
        if (!source) return ZF_ERR_ARG;

        outFloor.floorId = newFloorId;
        outFloor.floorName = newFloorName;
        outFloor.floorIndex = source->floorIndex + 1;
        outFloor.elevation_m = source->elevation_m + source->height_m;
        outFloor.height_m = source->height_m;
        outFloor.netHeight_m = source->netHeight_m;
        outFloor.referenceFloorId = source->floorId;
        outFloor.isStandardFloor = false;

        std::string prefix = options.idPrefix.empty() ?
            (newFloorId + "_") : options.idPrefix;

        // 复制墙体
        if (options.cloneWalls) {
            for (const auto& w : source->walls) {
                Wall nw = w;
                nw.wallId = prefix + w.wallId;
                for (auto& p : nw.points) {
                    p.x += options.offsetX;
                    p.y += options.offsetY;
                }
                outFloor.walls.push_back(nw);
            }
        }

        // 复制门
        if (options.cloneDoors) {
            for (const auto& d : source->doors) {
                Door nd = d;
                nd.doorId = prefix + d.doorId;
                nd.position.x += options.offsetX;
                nd.position.y += options.offsetY;
                outFloor.doors.push_back(nd);
            }
        }

        // 复制窗
        if (options.cloneWindows) {
            for (const auto& win : source->windows) {
                Window nw = win;
                nw.windowId = prefix + win.windowId;
                nw.position.x += options.offsetX;
                nw.position.y += options.offsetY;
                outFloor.windows.push_back(nw);
            }
        }

        // 复制房间
        if (options.cloneRooms) {
            for (const auto& r : source->rooms) {
                Room nr = r;
                nr.roomId = prefix + r.roomId;
                for (auto& p : nr.polygon) {
                    p.x += options.offsetX;
                    p.y += options.offsetY;
                }
                outFloor.rooms.push_back(nr);
            }
        }

        // 复制器件
        if (options.cloneDevices) {
            // 建立旧ID到新ID的映射
            std::map<std::string, std::string> idMap;
            for (const auto& dev : source->devices) {
                idMap[dev.instanceId] = prefix + dev.instanceId;
            }

            for (const auto& dev : source->devices) {
                DeviceInstance nd = dev;
                nd.instanceId = prefix + dev.instanceId;
                nd.position.x += options.offsetX;
                nd.position.y += options.offsetY;

                // 更新连接关系中的目标ID
                for (auto& conn : nd.connections) {
                    auto it = idMap.find(conn.targetInstanceId);
                    if (it != idMap.end()) {
                        conn.targetInstanceId = it->second;
                    }
                    if (!conn.cableSegmentId.empty()) {
                        conn.cableSegmentId = prefix + conn.cableSegmentId;
                    }
                }
                outFloor.devices.push_back(nd);
            }
        }

        // 复制线缆
        if (options.cloneCables) {
            for (const auto& cab : source->cables) {
                CableSegment nc = cab;
                nc.segmentId = prefix + cab.segmentId;
                outFloor.cables.push_back(nc);
            }
        }

        // 底图设置（不复制图片，但保留比例设置）
        if (options.cloneBackground) {
            outFloor.backgroundImage = source->backgroundImage;
            outFloor.drawingScale = source->drawingScale;
            outFloor.backgroundOpacity = source->backgroundOpacity;
            outFloor.backgroundGrayscale = source->backgroundGrayscale;
        }

        return ZF_ERR_OK;
    }

    // 同步更新：将标准层的变更同步到所有引用层
    int syncToReferences(const Floor* standard,
                         std::vector<Floor>& referenceFloors,
                         const CloneOptions& options) {
        if (!standard) return ZF_ERR_ARG;
        int synced = 0;

        for (auto& ref : referenceFloors) {
            if (ref.referenceFloorId == standard->floorId && !ref.isStandardFloor) {
                Floor temp;
                int result = cloneFromStandard(standard, ref.floorId, ref.floorName,
                                               options, temp);
                if (result == ZF_ERR_OK) {
                    // 保留楼层基本信息，替换内容
                    if (options.cloneWalls) ref.walls = temp.walls;
                    if (options.cloneDoors) ref.doors = temp.doors;
                    if (options.cloneWindows) ref.windows = temp.windows;
                    if (options.cloneRooms) ref.rooms = temp.rooms;
                    if (options.cloneDevices) ref.devices = temp.devices;
                    if (options.cloneCables) ref.cables = temp.cables;
                    synced++;
                }
            }
        }
        return synced;
    }

    // 断开引用关系（将引用层转为独立楼层）
    int detachReference(Floor* floor) {
        if (!floor) return ZF_ERR_ARG;
        floor->referenceFloorId.clear();
        return ZF_ERR_OK;
    }

    // 统计项目中的标准层和引用层
    struct FloorCloneStats {
        int standardCount{0};
        int referenceCount{0};
        int independentCount{0};
        std::vector<std::string> standardFloorIds;
        std::map<std::string, std::vector<std::string>> referenceMap;
    };

    FloorCloneStats analyzeProject(const std::vector<Floor>& floors) const {
        FloorCloneStats stats;
        for (const auto& f : floors) {
            if (f.isStandardFloor) {
                stats.standardCount++;
                stats.standardFloorIds.push_back(f.floorId);
            } else if (!f.referenceFloorId.empty()) {
                stats.referenceCount++;
                stats.referenceMap[f.referenceFloorId].push_back(f.floorId);
            } else {
                stats.independentCount++;
            }
        }
        return stats;
    }

    // 比较两个楼层的器件数量差异
    struct FloorDiff {
        int wallDiff{0};
        int deviceDiff{0};
        int cableDiff{0};
        int roomDiff{0};
    };

    FloorDiff compareFloors(const Floor* a, const Floor* b) const {
        FloorDiff diff;
        if (!a || !b) return diff;
        diff.wallDiff = (int)b->walls.size() - (int)a->walls.size();
        diff.deviceDiff = (int)b->devices.size() - (int)a->devices.size();
        diff.cableDiff = (int)b->cables.size() - (int)a->cables.size();
        diff.roomDiff = (int)b->rooms.size() - (int)a->rooms.size();
        return diff;
    }
};

} // namespace zf
