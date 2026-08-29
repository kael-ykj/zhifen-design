cmake_minimum_required(VERSION 3.16)
project(ZhiFenDesign VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt
if(WIN32)
    # Windows下从环境变量或默认路径查找Qt
    if(NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH $ENV{QT_DIR})
    endif()
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL PrintSupport REQUIRED)

# 源文件
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/cad/document.cpp
    src/cad/cadscene.cpp
    src/cad/cadview.cpp
    src/cad/undocommands.cpp
    src/entities/caditem.cpp
    src/entities/lineitem.cpp
    src/entities/circleitem.cpp
    src/entities/arcitem.cpp
    src/entities/polylineitem.cpp
    src/entities/rectangleitem.cpp
    src/entities/textitem.cpp
    src/entities/dimensionitem.cpp
    src/devices/deviceitem.cpp
    src/tools/tool.cpp
    src/tools/selecttool.cpp
    src/tools/linetool.cpp
    src/tools/circletool.cpp
    src/tools/arctool.cpp
    src/tools/polylinetool.cpp
    src/tools/rectangletool.cpp
    src/tools/texttool.cpp
    src/tools/dimensiontool.cpp
    src/tools/copytool.cpp
    src/tools/rotatetool.cpp
    src/tools/scaletool.cpp
    src/tools/mirrortool.cpp
    src/tools/explodetool.cpp
    src/tools/offsettool.cpp
    src/tools/querytool.cpp
    src/tools/feedertool.cpp
    src/entities/feederitem.cpp
    src/engine/system_diagram_generator.cpp
    src/engine/report_engine.cpp
    src/import/dxf_importer.cpp
    src/engine/coverage_simulator.cpp
    src/tools/batch_importer.cpp
    src/core/floor_manager.cpp
    src/plugins/core_api.cpp
    src/plugins/plugin_manager.cpp
    src/plugins/batch_rename_plugin.cpp
    src/tools/devicetool.cpp
    src/engine/route_planner.cpp
    src/engine/power_balance_optimizer.cpp
    src/tools/special_design_tools.cpp
    src/entities/dimension_item.cpp
    src/core/sheet_set_manager.cpp
    src/core/block_manager.cpp
    src/core/layout_manager.cpp
    src/core/version_manager.cpp
    src/core/change_review_manager.cpp
    src/core/network_planning_tools.cpp
    src/tools/pantool.cpp
    src/core/performance_manager.cpp
    src/tools/zoomtool.cpp
    src/core/format_converter.cpp
    src/snap/snapmanager.cpp
    src/core/professional_symbols.cpp
    src/io/dxfreader.cpp
    src/io/dxfwriter.cpp
    src/io/projectio.cpp
    src/widgets/commandline.cpp
    src/widgets/layerpanel.cpp
    src/widgets/propertypanel.cpp
    src/widgets/layerdialog.cpp
    src/widgets/devicepanel.cpp
)

set(HEADERS
    src/mainwindow.h
    src/cad/document.h
    src/cad/cadscene.h
    src/cad/cadview.h
    src/cad/undocommands.h
    src/entities/caditem.h
    src/entities/lineitem.h
    src/entities/circleitem.h
    src/entities/arcitem.h
    src/entities/polylineitem.h
    src/entities/rectangleitem.h
    src/entities/textitem.h
    src/entities/dimensionitem.h
    src/devices/deviceitem.h
    src/tools/tool.h
    src/tools/selecttool.h
    src/tools/linetool.h
    src/tools/circletool.h
    src/tools/arctool.h
    src/tools/polylinetool.h
    src/tools/rectangletool.h
    src/tools/texttool.h
    src/tools/dimensiontool.h
    src/tools/copytool.h
    src/tools/rotatetool.h
    src/tools/scaletool.h
    src/tools/mirrortool.h
    src/tools/explodetool.h
    src/tools/offsettool.h
    src/tools/querytool.h
    src/tools/feedertool.h
    src/engine/system_diagram_generator.h
    src/engine/report_engine.h
    src/import/dxf_importer.h
    src/engine/coverage_simulator.h
    src/plugins/iplugin.h
    src/plugins/core_api.h
    src/plugins/plugin_manager.h
    src/plugins/batch_rename_plugin.h
    src/tools/batch_importer.h
    src/engine/route_planner.h
    src/engine/power_balance_optimizer.h
    src/tools/special_design_tools.h
    src/entities/dimension_item.h
    src/core/block_manager.h
    src/core/layout_manager.h
    src/core/version_manager.h
    src/core/change_review_manager.h
    src/core/sheet_set_manager.h
    src/core/network_planning_tools.h
    src/core/floor_manager.h
    src/entities/feederitem.h
    src/core/performance_manager.h
    src/tools/devicetool.h
    src/tools/pantool.h
    src/core/format_converter.h
    src/core/professional_symbols.h
    src/tools/zoomtool.h
    src/snap/snapmanager.h
    src/io/dxfreader.h
    src/io/dxfwriter.h
    src/widgets/commandline.h
    src/widgets/layerpanel.h
    src/widgets/propertypanel.h
    src/widgets/layerdialog.h
    src/widgets/devicepanel.h
)

# 资源文件
qt5_add_resources(RESOURCES resources/resources.qrc)

add_executable(ZhiFenDesign ${SOURCES} ${HEADERS} ${RESOURCES})

target_include_directories(ZhiFenDesign PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/cad
    ${CMAKE_SOURCE_DIR}/src/entities
    ${CMAKE_SOURCE_DIR}/src/tools
    ${CMAKE_SOURCE_DIR}/src/snap
    ${CMAKE_SOURCE_DIR}/src/io
    ${CMAKE_SOURCE_DIR}/src/widgets
    ${CMAKE_SOURCE_DIR}/src/devices
)

target_link_libraries(ZhiFenDesign PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::PrintSupport
)

# Linux本地验证：忽略共享库中ICU版本不匹配的未定义符号
if(UNIX AND NOT APPLE)
    target_link_options(ZhiFenDesign PRIVATE "-Wl,--unresolved-symbols=ignore-in-shared-libs")
endif()

# Windows特定设置
if(WIN32)
    set_target_properties(ZhiFenDesign PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "智分Design"
    )
    # 设置图标
    if(EXISTS ${CMAKE_SOURCE_DIR}/resources/icons/app.ico)
        target_sources(ZhiFenDesign PRIVATE resources/icons/app.rc)
    endif()
endif()

# 安装规则
install(TARGETS ZhiFenDesign DESTINATION bin)
