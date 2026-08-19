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

using namespace zf;

void printSeparator() {
    std::cout << "========================================" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "智分Design V3.1.0-FINAL (P0内核演示版)" << std::endl;
    std::cout << "仅供个人学习研究使用" << std::endl;
    printSeparator();

    // 1. 初始化模式控制层
    std::cout << "[1/7] 初始化模式控制层..." << std::endl;
    ModeControlLayer modeLayer;
    modeLayer.init();
    ModeManager* modeMgr = modeLayer.modeManager.get();
    std::cout << "  默认模式: " << (modeMgr->getGlobalWorkMode() == WorkMode::SKETCH_MODE ? "草图模式" : "正式模式") << std::endl;
    std::cout << "  草图模式重型计算权限: " << zfErrorString(modeMgr->checkHeavyComputePermission()) << std::endl;

    // 2. 初始化器件库
    std::cout << "[2/7] 加载器件库..." << std::endl;
    DeviceLibrary devLib;
    devLib.loadDefaultLibrary();
    std::cout << "  器件模板数量: " << devLib.modelCount() << std::endl;
    auto ant = devLib.getModelById("ANT_OMNI_CEILING");
    if (ant) std::cout << "  全向吸顶天线: " << ant->displayName << " (" << ant->gain_dBi << "dBi)" << std::endl;

    // 3. 初始化Undo/Redo
    std::cout << "[3/7] 初始化双事务栈..." << std::endl;
    UndoRedoDoubleStack undoStack;
    std::cout << "  栈大小: " << undoStack.stackSize() << std::endl;

    // 4. 初始化图形引擎
    std::cout << "[4/7] 初始化图形引擎..." << std::endl;
    ZfScene scene;
    ZfView view(&scene);
    ZfLayerManager layerMgr;
    layerMgr.initDefaultLayers();
    std::cout << "  图层数量: " << layerMgr.layerCount() << std::endl;
    std::cout << "  当前缩放: " << view.zoomLevel() << std::endl;

    // 5. 初始化工具系统
    std::cout << "[5/7] 初始化工具系统..." << std::endl;
    ZfSnapEngine snap;
    ToolManager toolMgr;
    toolMgr.init(&snap, modeMgr, &devLib, &undoStack);
    toolMgr.activateTool(ToolType::PLACE_DEVICE);
    std::cout << "  当前工具: " << (toolMgr.activeTool() ? toolMgr.activeTool()->toolName() : "无") << std::endl;

    // 6. 模拟放置器件
    std::cout << "[6/7] 模拟放置器件与复制..." << std::endl;
    Project project;
    project.projectId = "DEMO_001";
    project.projectName = "演示工程";
    Floor floor;
    floor.floorId = "F1";
    floor.floorName = "1F";

    auto* placeTool = dynamic_cast<ZfPlaceDeviceTool*>(toolMgr.activeTool());
    if (placeTool) {
        placeTool->setCurrentModelId("ANT_OMNI_CEILING");
        DeviceInstance dev1 = placeTool->placeDevice({100, 200});
        floor.devices.push_back(dev1);
        std::cout << "  放置天线: " << dev1.instanceId << " at (" << dev1.position.x << "," << dev1.position.y << ")" << std::endl;
        std::cout << "  草图占位标记: " << (dev1.sketchPlaceholder ? "是" : "否") << std::endl;
    }

    // 模拟轻量复制
    toolMgr.activateTool(ToolType::EDIT);
    auto* editTool = dynamic_cast<ZfEditTool*>(toolMgr.activeTool());
    if (editTool && !floor.devices.empty()) {
        editTool->setCopyMode(CopyDuplicateMode::LIGHT_COPY);
        std::vector<std::string> ids = {floor.devices[0].instanceId};
        editTool->doDuplicate(floor.devices, ids, {50, 0});
        std::cout << "  轻量复制后器件数: " << floor.devices.size() << std::endl;
        if (floor.devices.size() > 1) {
            std::cout << "  复制件备注为空: " << (floor.devices[1].userNote.empty() ? "是(轻量复制正确)" : "否") << std::endl;
        }
    }
    project.floors.push_back(floor);

    // 7. 测试模式切换与导出
    std::cout << "[7/7] 测试模式切换与导出..." << std::endl;
    DwgExporter exporter;
    exporter.setModeManager(modeMgr);

    int sketchResult = exporter.exportDwg("demo_sketch.dwg", &project, DwgExportMode::SKETCH_EXPORT);
    std::cout << "  草图导出结果: " << zfErrorString(sketchResult) << std::endl;

    int formalResult = exporter.exportDwg("demo_formal.dwg", &project, DwgExportMode::FORMAL_EXPORT);
    std::cout << "  正式导出结果(草图模式下): " << zfErrorString(formalResult) << std::endl;

    modeMgr->setGlobalWorkMode(WorkMode::FORMAL_MODE, "user");
    std::cout << "  切换到正式模式后审计日志数: " << modeMgr->getAuditLog().size() << std::endl;

    int formalResult2 = exporter.exportDwg("demo_formal.dwg", &project, DwgExportMode::FORMAL_EXPORT);
    std::cout << "  正式导出结果(正式模式下): " << zfErrorString(formalResult2) << std::endl;

    printSeparator();
    std::cout << "P0内核演示完成！所有核心模块运行正常。" << std::endl;
    std::cout << "审计日志记录: " << modeMgr->getAuditLog().size() << " 条" << std::endl;
    std::cout << "按回车键退出..." << std::endl;
    std::cin.get();
    return 0;
}
