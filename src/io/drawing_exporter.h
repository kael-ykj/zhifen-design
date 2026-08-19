#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "mode_control/mode_control_layer.h"
#include "engine/system_diagram_engine.h"

namespace zf {

// DXF 生成器（纯文本 AutoCAD 交换格式）
class DxfWriter {
public:
    DxfWriter() { reset(); }

    void reset() {
        m_ss.str("");
        m_ss.clear();
    }

    // 写 DXF 对
    void writePair(int code, const std::string& value) {
        m_ss << code << "\n" << value << "\n";
    }
    void writePair(int code, double value) {
        m_ss << code << "\n" << std::fixed << std::setprecision(4) << value << "\n";
    }
    void writePair(int code, int value) {
        m_ss << code << "\n" << value << "\n";
    }

    // 开始 SECTION
    void beginSection(const std::string& name) {
        writePair(0, "SECTION");
        writePair(2, name);
    }
    void endSection() {
        writePair(0, "ENDSEC");
    }

    // 图层定义
    void addLayer(const std::string& name, int color) {
        writePair(0, "LAYER");
        writePair(2, name);
        writePair(70, 0);
        writePair(62, color);
        writePair(6, "CONTINUOUS");
    }

    // 直线
    void addLine(double x1, double y1, double x2, double y2, const std::string& layer = "0") {
        writePair(0, "LINE");
        writePair(8, layer);
        writePair(10, x1);
        writePair(20, y1);
        writePair(11, x2);
        writePair(21, y2);
    }

    // 圆
    void addCircle(double cx, double cy, double r, const std::string& layer = "0") {
        writePair(0, "CIRCLE");
        writePair(8, layer);
        writePair(10, cx);
        writePair(20, cy);
        writePair(40, r);
    }

    // 矩形（用4条线）
    void addRect(double x, double y, double w, double h, const std::string& layer = "0") {
        addLine(x, y, x + w, y, layer);
        addLine(x + w, y, x + w, y + h, layer);
        addLine(x + w, y + h, x, y + h, layer);
        addLine(x, y + h, x, y, layer);
    }

    // 文字
    void addText(double x, double y, double height, const std::string& text, const std::string& layer = "0") {
        writePair(0, "TEXT");
        writePair(8, layer);
        writePair(10, x);
        writePair(20, y);
        writePair(40, height);
        writePair(1, text);
    }

    // 完成 DXF
    std::string finish() {
        writePair(0, "EOF");
        return m_ss.str();
    }

    // 保存到文件
    int saveToFile(const std::string& filePath) {
        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return ZF_ERR_IO;
        ofs << finish();
        ofs.close();
        return ZF_ERR_OK;
    }

private:
    std::stringstream m_ss;
};

// 材料表条目
struct MaterialItem {
    std::string modelId;
    std::string displayName;
    std::string unit;
    int quantity{0};
    std::string remark;
};

// 出图引擎
class DrawingExporter {
public:
    DrawingExporter() = default;

    void setModeManager(ModeManager* mgr) { m_modeMgr = mgr; }

    // 导出系统图为 DXF
    int exportSystemDiagramDxf(const std::string& filePath, const SystemDiagram& diagram) {
        if (filePath.empty()) return ZF_ERR_ARG_EMPTY;

        bool isSketch = (m_modeMgr && m_modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE);

        DxfWriter dxf;

        // HEADER
        dxf.beginSection("HEADER");
        dxf.writePair(9, "$ACADVER");
        dxf.writePair(1, "AC1009");  // AutoCAD R12 兼容
        dxf.endSection();

        // TABLES - 图层
        dxf.beginSection("TABLES");
        dxf.writePair(0, "TABLE");
        dxf.writePair(2, "LAYER");
        dxf.writePair(70, 4);
        dxf.addLayer("0", 7);       // 白色 - 器件
        dxf.addLayer("WIRE", 3);    // 绿色 - 线缆
        dxf.addLayer("TEXT", 5);    // 蓝色 - 标注
        dxf.addLayer("FRAME", 8);   // 灰色 - 图框
        dxf.writePair(0, "ENDTAB");
        dxf.endSection();

        // ENTITIES
        dxf.beginSection("ENTITIES");

        // 图框
        dxf.addRect(50, 50, 800, 600, "FRAME");
        dxf.addText(400, 620, 8, "System Diagram - " + diagram.floorId, "FRAME");

        // 绘制节点
        for (const auto& node : diagram.nodes) {
            double x = node.layoutPos.x;
            double y = node.layoutPos.y;

            // 根据类型画不同符号
            if (node.type == NodeType::SOURCE) {
                dxf.addRect(x - 20, y - 15, 40, 30, "0");
                dxf.addText(x - 15, y + 20, 5, node.label, "TEXT");
            } else if (node.type == NodeType::ANTENNA) {
                dxf.addCircle(x, y, 12, "0");
                dxf.addLine(x - 8, y, x + 8, y, "0");
                dxf.addText(x - 15, y + 20, 4, node.label, "TEXT");
            } else {
                // 功分器/耦合器 - 三角形
                dxf.addLine(x - 15, y - 12, x + 15, y, "0");
                dxf.addLine(x + 15, y, x - 15, y + 12, "0");
                dxf.addLine(x - 15, y - 12, x - 15, y + 12, "0");
                dxf.addText(x - 15, y + 20, 4, node.label, "TEXT");
            }
        }

        // 绘制连接线
        for (const auto& link : diagram.links) {
            const SystemNode* from = findNode(diagram, link.fromNodeId);
            const SystemNode* to = findNode(diagram, link.toNodeId);
            if (from && to) {
                dxf.addLine(from->layoutPos.x, from->layoutPos.y,
                           to->layoutPos.x, to->layoutPos.y, "WIRE");
                // 标注损耗（正式模式）
                if (!isSketch) {
                    double mx = (from->layoutPos.x + to->layoutPos.x) / 2;
                    double my = (from->layoutPos.y + to->layoutPos.y) / 2;
                    std::string label = std::to_string(link.loss_dB).substr(0, 4) + "dB";
                    dxf.addText(mx, my + 5, 3, label, "TEXT");
                }
            }
        }

        dxf.endSection();

        return dxf.saveToFile(filePath);
    }

    // 导出平面图为 DXF
    int exportFloorPlanDxf(const std::string& filePath, const Floor* floor) {
        if (!floor || filePath.empty()) return ZF_ERR_ARG;

        DxfWriter dxf;

        dxf.beginSection("HEADER");
        dxf.writePair(9, "$ACADVER");
        dxf.writePair(1, "AC1009");
        dxf.endSection();

        dxf.beginSection("TABLES");
        dxf.writePair(0, "TABLE");
        dxf.writePair(2, "LAYER");
        dxf.writePair(70, 3);
        dxf.addLayer("DEVICE", 1);   // 红色 - 器件
        dxf.addLayer("CABLE", 3);    // 绿色 - 线缆
        dxf.addLayer("WALL", 8);     // 灰色 - 墙体
        dxf.writePair(0, "ENDTAB");
        dxf.endSection();

        dxf.beginSection("ENTITIES");

        // 绘制墙体（多边形点集）
        for (const auto& wall : floor->walls) {
            for (size_t i = 0; i + 1 < wall.points.size(); i++) {
                dxf.addLine(wall.points[i].x, wall.points[i].y,
                           wall.points[i+1].x, wall.points[i+1].y, "WALL");
            }
            // 闭合
            if (wall.points.size() > 2) {
                dxf.addLine(wall.points.back().x, wall.points.back().y,
                           wall.points[0].x, wall.points[0].y, "WALL");
            }
        }

        // 绘制器件
        for (const auto& dev : floor->devices) {
            dxf.addCircle(dev.position.x, dev.position.y, 8, "DEVICE");
            dxf.addText(dev.position.x + 10, dev.position.y, 4, dev.instanceId, "DEVICE");
        }

        // 绘制线缆（简化：用器件位置连线）
        for (const auto& dev : floor->devices) {
            for (const auto& conn : dev.connections) {
                const DeviceInstance* target = findDeviceById(floor, conn.targetInstanceId);
                if (target) {
                    dxf.addLine(dev.position.x, dev.position.y,
                               target->position.x, target->position.y, "CABLE");
                }
            }
        }

        dxf.endSection();
        return dxf.saveToFile(filePath);
    }

    // 生成材料表
    std::vector<MaterialItem> generateMaterialList(const Project* project) {
        std::vector<MaterialItem> items;
        if (!project) return items;

        std::map<std::string, int> countMap;
        std::map<std::string, DeviceModel> modelMap;

        // 统计所有楼层的器件
        for (const auto& floor : project->floors) {
            for (const auto& dev : floor.devices) {
                countMap[dev.modelId]++;
            }
            for (const auto& cab : floor.cables) {
                countMap[cab.modelId]++;  // 线缆按段统计
            }
        }

        // 查找器件模板信息
        for (const auto& m : project->deviceLibrary) {
            modelMap[m.modelId] = m;
        }

        for (const auto& [modelId, qty] : countMap) {
            MaterialItem item;
            item.modelId = modelId;
            item.quantity = qty;
            auto it = modelMap.find(modelId);
            if (it != modelMap.end()) {
                item.displayName = it->second.displayName;
                // 根据类别推断单位
                if (it->second.category == DeviceCategory::CABLE) {
                    item.unit = "米";
                } else {
                    item.unit = "个";
                }
            } else {
                item.displayName = modelId;
                item.unit = "个";
            }
            items.push_back(item);
        }

        return items;
    }

    // 导出材料表为 CSV
    int exportMaterialCsv(const std::string& filePath, const Project* project) {
        if (!project || filePath.empty()) return ZF_ERR_ARG;

        auto items = generateMaterialList(project);

        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return ZF_ERR_IO;

        ofs << "序号,型号,名称,单位,数量,备注\n";
        for (size_t i = 0; i < items.size(); i++) {
            ofs << (i + 1) << ","
                << items[i].modelId << ","
                << items[i].displayName << ","
                << items[i].unit << ","
                << items[i].quantity << ","
                << items[i].remark << "\n";
        }
        ofs.close();
        return ZF_ERR_OK;
    }

private:
    ModeManager* m_modeMgr{nullptr};

    const SystemNode* findNode(const SystemDiagram& diagram, const std::string& id) {
        for (const auto& n : diagram.nodes) {
            if (n.nodeId == id) return &n;
        }
        return nullptr;
    }

    const DeviceInstance* findDeviceById(const Floor* floor, const std::string& id) {
        for (const auto& d : floor->devices) {
            if (d.instanceId == id) return &d;
        }
        return nullptr;
    }
};

} // namespace zf
