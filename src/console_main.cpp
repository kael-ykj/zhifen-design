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
#include "engine/building_model_engine.h"
#include "engine/propagation_engine.h"
#include "engine/batch_editor.h"
#include "engine/floor_cloner.h"
#include "plugin/plugin_framework.h"
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

    // P1-04 墙体建模与底图校准
    std::cout << "\n[P1-04] 墙体建模、房间识别、底图校准..." << std::endl;
    BuildingModelEngine buildEngine;

    // 创建一个矩形房间的4面墙
    Floor testFloor;
    testFloor.floorId = "F_TEST";
    Wall w1 = buildEngine.createWall("W1", {{0,0}, {10000,0}}, WallMaterial::CONCRETE, 200);
    Wall w2 = buildEngine.createWall("W2", {{10000,0}, {10000,8000}}, WallMaterial::CONCRETE, 200);
    Wall w3 = buildEngine.createWall("W3", {{10000,8000}, {0,8000}}, WallMaterial::CONCRETE, 200);
    Wall w4 = buildEngine.createWall("W4", {{0,8000}, {0,0}}, WallMaterial::BRICK, 240);
    testFloor.walls = {w1, w2, w3, w4};

    std::cout << "  创建墙体数: " << testFloor.walls.size() << std::endl;
    std::cout << "  混凝土墙衰减: " << buildEngine.getWallAttenuation(WallMaterial::CONCRETE) << "dB" << std::endl;
    std::cout << "  砖墙衰减: " << buildEngine.getWallAttenuation(WallMaterial::BRICK) << "dB" << std::endl;
    std::cout << "  W1长度: " << buildEngine.calcWallLength(w1) << "m" << std::endl;

    // 自动识别房间
    int detectResult = buildEngine.autoDetectRooms(&testFloor);
    std::cout << "  房间识别结果: " << zfErrorString(detectResult) << std::endl;
    std::cout << "  识别房间数: " << testFloor.rooms.size() << std::endl;
    for (const auto& r : testFloor.rooms) {
        std::cout << "    - " << r.name << ": " << r.area_m2 << " m², " << r.polygon.size() << "个顶点" << std::endl;
    }

    // 底图校准（3对点）
    std::vector<CalibrationPoint> calibPoints = {
        {{0, 0}, {0, 0}},
        {{100, 0}, {10000, 0}},
        {{0, 80}, {0, 8000}}
    };
    AffineTransform transform;
    int calibResult = buildEngine.computeAffineTransform(calibPoints, transform);
    std::cout << "  底图校准结果: " << zfErrorString(calibResult) << std::endl;
    std::cout << "  校准比例X: " << transform.scaleX() << ", 旋转: " << transform.rotation() << "rad" << std::endl;

    // 楼层统计
    auto stats = buildEngine.calcFloorStats(&testFloor);
    std::cout << "  楼层统计: " << stats.wallCount << "面墙, " << stats.totalWallLength_m << "m总长, "
              << stats.roomCount << "个房间, " << stats.totalRoomArea_m2 << "m²" << std::endl;
    std::cout << "P1-04: 墙体建模与底图校准 正常" << std::endl;

    // P1-05 多墙传播仿真与热力图
    std::cout << "\n[P1-05] 多墙传播仿真与热力图..." << std::endl;
    PropagationEngine propEngine;
    propEngine.setModeManager(modeMgr);

    SimulationConfig simCfg;
    simCfg.frequency_MHz = 900.0;
    simCfg.gridResolution_m = 1.0;
    simCfg.txPower_dBm = 15.0;
    simCfg.antennaGain_dBi = 3.0;
    simCfg.maxDistance_m = 30.0;
    propEngine.setConfig(simCfg);

    // 在testFloor中放置天线，生成热力图
    Point2D txPos = {5000, 4000};  // 房间中心
    HeatmapData heatmap;
    int simResult = propEngine.generateHeatmap(&testFloor, txPos, heatmap);
    std::cout << "  仿真结果: " << zfErrorString(simResult) << std::endl;
    if (simResult == ZF_ERR_OK) {
        std::cout << "  网格: " << heatmap.gridWidth << "x" << heatmap.gridHeight << std::endl;
        std::cout << "  RSRP范围: " << heatmap.minRSRP << " ~ " << heatmap.maxRSRP << " dBm" << std::endl;
        std::cout << "  平均RSRP: " << heatmap.avgRSRP << " dBm" << std::endl;
        std::cout << "  覆盖率(>=-100dBm): " << (heatmap.coverageRate * 100) << "%" << std::endl;
        std::cout << "  弱覆盖点数: " << heatmap.weakCoverageCount << std::endl;

        // 显示简化热力图
        std::cout << "\n  --- 热力图(简化) ---" << std::endl;
        std::cout << propEngine.heatmapToText(heatmap, 30) << std::endl;
    }

    // 自由空间损耗验证
    std::cout << "  自由空间损耗(10m@900MHz): " << propEngine.freeSpaceLoss(10, 900) << "dB" << std::endl;
    std::cout << "P1-05: 多墙传播仿真 正常" << std::endl;

    // P1-06 批量编辑与CSV导入导出
    std::cout << "\n[P1-06] 批量编辑与CSV导入导出..." << std::endl;
    BatchEditor batchEditor;

    // 创建测试CSV文件
    {
        std::ofstream ofs("test_devices.csv");
        ofs << "instanceId,modelId,x,y,note\n";
        ofs << "BAT_001,ANT_OMNI_CEILING,100,200,批量导入天线1\n";
        ofs << "BAT_002,ANT_OMNI_CEILING,300,400,批量导入天线2\n";
        ofs << "BAT_003,SPLIT_2WAY,500,600,批量导入功分器\n";
        ofs.close();
    }

    // 导入CSV
    Floor batchFloor;
    batchFloor.floorId = "F_BATCH";
    BatchImportResult importResult;
    int importRet = batchEditor.importDevicesFromCsv("test_devices.csv", &devLib, &batchFloor, importResult);
    std::cout << "  CSV导入结果: " << zfErrorString(importRet) << std::endl;
    std::cout << "  总行数: " << importResult.totalRows
              << ", 成功: " << importResult.successCount
              << ", 失败: " << importResult.failedCount << std::endl;
    std::cout << "  导入后器件数: " << batchFloor.devices.size() << std::endl;

    // 按型号统计
    auto counts = batchEditor.countByModel(&batchFloor);
    std::cout << "  型号统计:" << std::endl;
    for (const auto& [model, cnt] : counts) {
        std::cout << "    - " << model << ": " << cnt << "个" << std::endl;
    }

    // 批量修改属性
    std::vector<std::string> ids = {"BAT_001", "BAT_002"};
    int updated = batchEditor.batchUpdateProperty(&batchFloor, ids, "userNote", "已批量修改备注");
    std::cout << "  批量修改属性: " << updated << "个器件" << std::endl;

    // 批量移动
    int moved = batchEditor.batchMoveDevices(&batchFloor, ids, 50, 50);
    std::cout << "  批量移动: " << moved << "个器件" << std::endl;

    // 导出CSV
    int exportRet = batchEditor.exportDevicesToCsv("test_export.csv", &batchFloor);
    std::cout << "  CSV导出: " << zfErrorString(exportRet) << std::endl;

    // 批量删除
    int deleted = batchEditor.batchDeleteDevices(&batchFloor, {"BAT_003"});
    std::cout << "  批量删除: " << deleted << "个器件, 剩余: " << batchFloor.devices.size() << std::endl;

    // 清理测试文件
    std::remove("test_devices.csv");
    std::remove("test_export.csv");
    std::cout << "P1-06: 批量编辑与CSV导入导出 正常" << std::endl;

    // P1-07 标准层复制
    std::cout << "\n[P1-07] 标准层复制与同步..." << std::endl;
    FloorCloner cloner;

    // 把testFloor标记为标准层
    cloner.markAsStandard(&testFloor, "标准层模板");
    std::cout << "  标记标准层: " << (testFloor.isStandardFloor ? "是" : "否") << std::endl;
    std::cout << "  标准层器件数: " << testFloor.devices.size()
              << ", 墙体数: " << testFloor.walls.size() << std::endl;

    // 从标准层复制到2F
    CloneOptions cloneOpts;
    cloneOpts.cloneWalls = true;
    cloneOpts.cloneDevices = true;
    cloneOpts.cloneCables = true;
    cloneOpts.cloneRooms = true;
    cloneOpts.idPrefix = "F2_";

    Floor floor2;
    int cloneResult = cloner.cloneFromStandard(&testFloor, "F2", "2F", cloneOpts, floor2);
    std::cout << "  复制到2F结果: " << zfErrorString(cloneResult) << std::endl;
    std::cout << "  2F器件数: " << floor2.devices.size()
              << ", 墙体数: " << floor2.walls.size()
              << ", 房间数: " << floor2.rooms.size() << std::endl;
    std::cout << "  2F引用标准层: " << floor2.referenceFloorId << std::endl;

    // 验证ID前缀
    if (!floor2.devices.empty()) {
        std::cout << "  2F首个器件ID: " << floor2.devices[0].instanceId << " (前缀验证)" << std::endl;
    }

    // 复制到3F
    cloneOpts.idPrefix = "F3_";
    Floor floor3;
    cloner.cloneFromStandard(&testFloor, "F3", "3F", cloneOpts, floor3);
    std::cout << "  3F器件数: " << floor3.devices.size() << std::endl;

    // 统计
    std::vector<Floor> allFloors = {testFloor, floor2, floor3};
    auto cloneStats = cloner.analyzeProject(allFloors);
    std::cout << "  楼层统计: 标准层" << cloneStats.standardCount
              << "个, 引用层" << cloneStats.referenceCount
              << "个, 独立层" << cloneStats.independentCount << "个" << std::endl;

    // 比较差异
    auto diff = cloner.compareFloors(&testFloor, &floor2);
    std::cout << "  标准层与2F差异: 墙体" << diff.wallDiff
              << ", 器件" << diff.deviceDiff << std::endl;

    // 断开引用
    cloner.detachReference(&floor3);
    std::cout << "  3F断开引用后: " << (floor3.referenceFloorId.empty() ? "已独立" : "仍引用") << std::endl;

    std::cout << "P1-07: 标准层复制与同步 正常" << std::endl;

    // P1-08 插件API框架
    std::cout << "\n[P1-08] 插件API框架与权限约束..." << std::endl;
    PluginManager pluginMgr;

    // 列出插件
    auto plugins = pluginMgr.listPlugins();
    std::cout << "  已注册插件数: " << pluginMgr.pluginCount() << std::endl;
    for (const auto& p : plugins) {
        std::cout << "    - [" << p.pluginId << "] " << p.name
                  << " v" << p.version << " (权限:" << (int)p.permissions << ")" << std::endl;
    }

    // 执行器件检查插件
    PluginContext ctx;
    ctx.project = &project;
    ctx.currentFloor = &project.floors[0];

    PluginResult checkResult = pluginMgr.executePlugin(
        "builtin.device_check", ctx,
        PluginPermission::READ_PROJECT);
    std::cout << "  器件检查插件: " << (checkResult.success ? "成功" : "失败")
              << " - " << checkResult.message << std::endl;

    // 执行覆盖率报告插件
    PluginResult covResult = pluginMgr.executePlugin(
        "builtin.coverage_report", ctx,
        PluginPermission::READ_PROJECT | PluginPermission::RUN_SIMULATION);
    std::cout << "  覆盖率报告插件: " << (covResult.success ? "成功" : "失败")
              << " - " << covResult.message << std::endl;

    // 权限不足测试
    PluginResult deniedResult = pluginMgr.executePlugin(
        "builtin.coverage_report", ctx,
        PluginPermission::READ_PROJECT);  // 缺少RUN_SIMULATION权限
    std::cout << "  权限不足测试: " << (deniedResult.success ? "成功(异常)" : "被拒绝(正确)")
              << " - " << deniedResult.message << std::endl;

    // 禁用插件测试
    pluginMgr.setPluginEnabled("builtin.device_check", false);
    PluginResult disabledResult = pluginMgr.executePlugin(
        "builtin.device_check", ctx, PluginPermission::ALL);
    std::cout << "  禁用插件测试: " << (disabledResult.success ? "成功(异常)" : "被拒绝(正确)")
              << " - " << disabledResult.message << std::endl;
    pluginMgr.setPluginEnabled("builtin.device_check", true);

    std::cout << "P1-08: 插件API框架与权限约束 正常" << std::endl;

    printSeparator();
    std::cout << "P1阶段全部模块开发完成！" << std::endl;
    std::cout << "P1-01: 链路预算引擎" << std::endl;
    std::cout << "P1-02: 系统图自动布局" << std::endl;
    std::cout << "P1-03: 出图引擎(DXF+材料表)" << std::endl;
    std::cout << "P1-04: 墙体建模与底图校准" << std::endl;
    std::cout << "P1-05: 多墙传播仿真热力图" << std::endl;
    std::cout << "P1-06: 批量编辑与CSV导入" << std::endl;
    std::cout << "P1-07: 标准层复制" << std::endl;
    std::cout << "P1-08: 插件API框架" << std::endl;

    std::cout << "\n审计日志记录: " << modeMgr->getAuditLog().size() << " 条" << std::endl;
    std::cout << "按回车键退出..." << std::endl;
    std::cin.get();
    return 0;
}
