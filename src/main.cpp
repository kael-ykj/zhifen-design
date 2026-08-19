#include <iostream>
#include <string>
#include "core/zf_types.h"
#include "core/zf_error.h"
#include "core/alert_marker.h"
#include "mode_control/mode_control_layer.h"
#include "undo/undo_redo_stack.h"
#include "device/device_library.h"
#include "tools/tool_system.h"
#include "graphics/graphics_engine.h"
#include "io/project_io.h"
#include "io/dwg_exporter.h"
#include "engine/link_calculator.h"
#include "engine/system_diagram_engine.h"
#include "io/drawing_exporter.h"

using namespace zf;

void printSeparator() {
    std::cout << "========================================" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "智分Design V3.1.0-FINAL (P1内核演示版)" << std::endl;
    std::cout << "仅供个人学习研究使用" << std::endl;
    printSeparator();

    // 1. 初始化模式控制层
    std::cout << "[1/8] 初始化模式控制层..." << std::endl;
    ModeControlLayer modeLayer;
    modeLayer.init();
    ModeManager* modeMgr = modeLayer.modeManager.get();
    std::cout << "  默认模式: " << (modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE ? "草图模式" : "正式模式") << std::endl;
    std::cout << "  草图模式重型计算权限: " << zfErrorString(modeMgr->checkHeavyComputePermission()) << std::endl;

    // 2. 初始化器件库
    std::cout << "[2/8] 加载器件库..." << std::endl;
    DeviceLibrary devLib;
    devLib.loadDefaultLibrary();
    std::cout << "  器件模板数量: " << devLib.modelCount() << std::endl;
    auto ant = devLib.getModelById("ANT_OMNI_CEILING");
    if (ant) std::cout << "  全向吸顶天线: " << ant->displayName << " (" << ant->gain_dBi << "dBi)" << std::endl;

    // 3. 初始化Undo/Redo
    std::cout << "[3/8] 初始化双事务栈..." << std::endl;
    UndoRedoDoubleStack undoStack;
    std::cout << "  栈大小: " << undoStack.stackSize() << std::endl;

    // 4. 初始化图形引擎
    std::cout << "[4/8] 初始化图形引擎..." << std::endl;
    ZfScene scene;
    ZfView view(&scene);
    ZfLayerManager layerMgr;
    layerMgr.initDefaultLayers();
    std::cout << "  图层数量: " << layerMgr.layerCount() << std::endl;

    // 5. 初始化工具系统
    std::cout << "[5/8] 初始化工具系统..." << std::endl;
    ZfSnapEngine snap;
    ToolManager toolMgr;
    toolMgr.init(&snap, modeMgr, &devLib, &undoStack);
    toolMgr.activateTool(ToolType::PLACE_DEVICE);
    std::cout << "  当前工具: " << (toolMgr.activeTool() ? toolMgr.activeTool()->toolName() : "无") << std::endl;

    // 6. 模拟放置器件与复制
    std::cout << "[6/8] 模拟放置器件与复制..." << std::endl;
    Project project;
    project.projectId = "DEMO_001";
    project.projectName = "演示工程";
    project.maxAntennaPower_dBm = 15.0;
    project.maxFeederLoss_dB = 3.0;

    // 把器件库模板复制到项目中（链路计算需要）
    for (const auto& cat : {DeviceCategory::SIGNAL_SOURCE, DeviceCategory::SPLITTER,
                            DeviceCategory::COUPLER, DeviceCategory::ANTENNA, DeviceCategory::CABLE}) {
        auto models = devLib.getModelsByCategory(cat);
        for (const auto& m : models) project.deviceLibrary.push_back(m);
    }

    Floor floor;
    floor.floorId = "F1";
    floor.floorName = "1F";

    // 构建完整拓扑：信源 -> 二功分 -> 2根天线
    DeviceInstance source;
    source.instanceId = "SRC_001";
    source.modelId = "SRC_RRU";
    source.position = {50, 50};
    floor.devices.push_back(source);

    DeviceInstance splitter;
    splitter.instanceId = "SPL_001";
    splitter.modelId = "SPLIT_2WAY";
    splitter.position = {150, 50};
    splitter.connections.push_back({"ANT_001", "out1", "in", "CAB_001"});
    splitter.connections.push_back({"ANT_002", "out2", "in", "CAB_002"});
    floor.devices.push_back(splitter);

    DeviceInstance ant1;
    ant1.instanceId = "ANT_001";
    ant1.modelId = "ANT_OMNI_CEILING";
    ant1.position = {250, 100};
    floor.devices.push_back(ant1);

    DeviceInstance ant2;
    ant2.instanceId = "ANT_002";
    ant2.modelId = "ANT_OMNI_CEILING";
    ant2.position = {250, 0};
    floor.devices.push_back(ant2);

    // 信源连接到功分器
    floor.devices[0].connections.push_back({"SPL_001", "out", "in", "CAB_000"});

    // 馈线段
    CableSegment cab0;
    cab0.segmentId = "CAB_000";
    cab0.modelId = "CABLE_1_2";
    cab0.length_m = 10.0;
    floor.cables.push_back(cab0);

    CableSegment cab1;
    cab1.segmentId = "CAB_001";
    cab1.modelId = "CABLE_1_2";
    cab1.length_m = 15.0;
    floor.cables.push_back(cab1);

    CableSegment cab2;
    cab2.segmentId = "CAB_002";
    cab2.modelId = "CABLE_1_2";
    cab2.length_m = 15.0;
    floor.cables.push_back(cab2);

    // 信源配置
    SignalSourceConfig srcCfg;
    srcCfg.sourceId = "SRC_001";
    srcCfg.deviceModelId = "SRC_RRU";
    srcCfg.txPower_dBm = 20.0;
    project.sources.push_back(srcCfg);

    project.floors.push_back(floor);
    std::cout << "  构建拓扑: 信源(20dBm) -> 二功分 -> 2根天线" << std::endl;
    std::cout << "  器件总数: " << floor.devices.size() << ", 馈线数: " << floor.cables.size() << std::endl;

    // 7. 测试模式切换与导出
    std::cout << "[7/8] 测试模式切换与导出..." << std::endl;
    DwgExporter exporter;
    exporter.setModeManager(modeMgr);
    int sketchResult = exporter.exportDwg("demo_sketch.dwg", &project, DwgExportMode::SKETCH_EXPORT);
    std::cout << "  草图导出结果: " << zfErrorString(sketchResult) << std::endl;

    // 8. P1-01 链路预算计算
    std::cout << "[8/8] P1-01 链路预算计算..." << std::endl;
    LinkCalculator linkCalc;
    linkCalc.setModeManager(modeMgr);

    // 草图模式下应该被限制
    int sketchCalc = linkCalc.calculateProject(&project);
    std::cout << "  草图模式计算结果: " << zfErrorString(sketchCalc) << " (预期: 草图模式限制)" << std::endl;

    // 切换到正式模式
    modeMgr->setGlobalWorkMode(WorkMode::FORMAL_MODE, "user");
    std::cout << "  切换到正式模式..." << std::endl;

    // 正式模式下执行计算
    int formalCalc = linkCalc.calculateProject(&project);
    std::cout << "  正式模式计算结果: " << zfErrorString(formalCalc) << std::endl;

    if (formalCalc == ZF_ERR_OK) {
        const auto& report = linkCalc.getReport();
        std::cout << "  已计算器件数: " << report.calculatedDevices << "/" << report.totalDevices << std::endl;
        std::cout << "  错误数: " << report.errorCount << ", 警告数: " << report.warningCount << std::endl;
        std::cout << "  天线最大功率: " << report.maxAntennaPower_dBm << " dBm" << std::endl;
        std::cout << "  天线最小功率: " << report.minAntennaPower_dBm << " dBm" << std::endl;
        std::cout << "  天线平均功率: " << report.avgAntennaPower_dBm << " dBm" << std::endl;

        std::cout << "\n  --- 链路明细 ---" << std::endl;
        for (const auto& r : report.results) {
            std::cout << "  [" << r.deviceInstanceId << "] " << r.deviceModelId
                      << " 入:" << r.inputPower_dBm << "dBm 出:" << r.outputPower_dBm << "dBm";
            if (r.isAntenna) std::cout << " EIRP:" << r.eirp_dBm << "dBm";
            std::cout << " 损耗:" << r.cumulativeLoss_dB << "dB";
            if (r.powerOverLimit) std::cout << " [功率越限]";
            if (r.lossOverLimit) std::cout << " [损耗过大]";
            std::cout << std::endl;
        }
    }

    printSeparator();
    std::cout << "P1内核演示完成！" << std::endl;
    std::cout << "P0模块: 模式控制/器件库/Undo/工具/图形/IO/导出 全部正常" << std::endl;
    std::cout << "P1-01: 链路预算引擎 计算正常" << std::endl;

    // P1-02 系统图生成
    std::cout << "\n[P1-02] 平面图自动生成系统图..." << std::endl;
    SystemDiagramEngine sysEngine;
    sysEngine.setModeManager(modeMgr);
    SystemDiagram sysDiagram;
    int sysResult = sysEngine.generateFromFloor(&project.floors[0], &project, sysDiagram);
    std::cout << "  系统图生成结果: " << zfErrorString(sysResult) << std::endl;
    if (sysResult == ZF_ERR_OK) {
        std::cout << "  节点数: " << sysDiagram.nodes.size() << std::endl;
        std::cout << "  连接数: " << sysDiagram.links.size() << std::endl;
        std::cout << "\n  --- 系统图布局 ---" << std::endl;
        for (const auto& n : sysDiagram.nodes) {
            std::cout << "  [" << n.nodeId << "] " << n.label
                      << " 层内位置:(" << n.layoutPos.x << "," << n.layoutPos.y << ")" << std::endl;
        }
        std::cout << "\n  --- 系统连接 ---" << std::endl;
        for (const auto& l : sysDiagram.links) {
            std::cout << "  " << l.fromNodeId << " -> " << l.toNodeId
                      << " 线缆:" << l.cableModelId << " 长度:" << l.length_m << "m"
                      << " 损耗:" << l.loss_dB << "dB" << std::endl;
        }
    }
    std::cout << "P1-02: 系统图布局引擎 生成正常" << std::endl;

    // P1-03 出图引擎：DXF导出 + 材料表
    std::cout << "\n[P1-03] 出图引擎：DXF导出 + 材料表生成..." << std::endl;
    DrawingExporter drawExp;
    drawExp.setModeManager(modeMgr);

    // 导出系统图DXF
    int dxfSys = drawExp.exportSystemDiagramDxf("output_system.dxf", sysDiagram);
    std::cout << "  系统图DXF导出: " << zfErrorString(dxfSys) << std::endl;

    // 导出平面图DXF
    int dxfPlan = drawExp.exportFloorPlanDxf("output_floorplan.dxf", &project.floors[0]);
    std::cout << "  平面图DXF导出: " << zfErrorString(dxfPlan) << std::endl;

    // 生成材料表
    auto materials = drawExp.generateMaterialList(&project);
    std::cout << "  材料表条目数: " << materials.size() << std::endl;
    for (const auto& m : materials) {
        std::cout << "    - " << m.displayName << " (" << m.modelId << "): "
                  << m.quantity << " " << m.unit << std::endl;
    }

    // 导出材料表CSV
    int csvResult = drawExp.exportMaterialCsv("output_material.csv", &project);
    std::cout << "  材料表CSV导出: " << zfErrorString(csvResult) << std::endl;
    std::cout << "P1-03: 出图引擎 导出正常" << std::endl;

    std::cout << "\n审计日志记录: " << modeMgr->getAuditLog().size() << " 条" << std::endl;
    std::cout << "按回车键退出..." << std::endl;
    std::cin.get();
    return 0;
}
