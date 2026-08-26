cmake_minimum_required(VERSION 3.16)
project(ZhiFenDesign VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ===== 控制台内核版（无Qt依赖，始终可编译）=====
set(CONSOLE_SOURCES
    src/console_main.cpp
)
add_executable(ZhiFenDesign_Console ${CONSOLE_SOURCES})
target_include_directories(ZhiFenDesign_Console PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

# ===== Qt GUI版 =====
find_package(Qt6 QUIET COMPONENTS Widgets PrintSupport)
if(NOT Qt6_FOUND)
    find_package(Qt5 QUIET COMPONENTS Widgets PrintSupport)
endif()

set(QT_FOUND FALSE)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT_FOUND TRUE)
    message(STATUS "Qt6 found, building GUI version")
elseif(Qt5_FOUND)
    set(QT_VERSION_MAJOR 5)
    set(QT_FOUND TRUE)
    message(STATUS "Qt5 found, building GUI version")
endif()

if(QT_FOUND)
    set(GUI_SOURCES
        gui/main.cpp
        gui/mainwindow.cpp
        gui/canvaswidget.cpp
        gui/devicelistpanel.cpp
        gui/propertypanel.cpp
        gui/layerpanel.cpp
        gui/commandline.cpp
        gui/system_dialog.cpp
    )
    set(GUI_HEADERS
        gui/mainwindow.h
        gui/canvaswidget.h
        gui/devicelistpanel.h
        gui/propertypanel.h
        gui/layerpanel.h
        gui/commandline.h
        gui/system_dialog.h
    )
    add_executable(ZhiFenDesign WIN32 ${GUI_SOURCES} ${GUI_HEADERS})
    target_include_directories(ZhiFenDesign PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party
        ${CMAKE_SOURCE_DIR}/gui
    )
    if(QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(ZhiFenDesign PRIVATE Qt6::Widgets Qt6::PrintSupport)
    else()
        target_link_libraries(ZhiFenDesign PRIVATE Qt5::Widgets Qt5::PrintSupport)
    endif()
    set_target_properties(ZhiFenDesign PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
else()
    message(STATUS "Qt not found, only console version will be built")
endif()

# 编译选项
if(MSVC)
    target_compile_options(ZhiFenDesign_Console PRIVATE /W3 /utf-8)
    if(TARGET ZhiFenDesign)
        target_compile_options(ZhiFenDesign PRIVATE /W3 /utf-8)
    endif()
else()
    target_compile_options(ZhiFenDesign_Console PRIVATE -Wall -Wextra)
    if(TARGET ZhiFenDesign)
        target_compile_options(ZhiFenDesign PRIVATE -Wall -Wextra)
    endif()
endif()

# 版本定义
target_compile_definitions(ZhiFenDesign_Console PRIVATE
    ZF_VERSION_MAJOR=3
    ZF_VERSION_MINOR=1
    ZF_VERSION_PATCH=0
    ZF_VERSION_STRING="3.1.0-FINAL"
)
if(TARGET ZhiFenDesign)
    target_compile_definitions(ZhiFenDesign PRIVATE
        ZF_VERSION_MAJOR=3
        ZF_VERSION_MINOR=1
        ZF_VERSION_PATCH=0
        ZF_VERSION_STRING="3.1.0-FINAL"
    )
endif()

# 安装规则
install(TARGETS ZhiFenDesign_Console
    RUNTIME DESTINATION bin
    OPTIONAL
)
if(TARGET ZhiFenDesign)
    install(TARGETS ZhiFenDesign
        RUNTIME DESTINATION bin
    )
endif()
install(FILES README.md
    DESTINATION .
    OPTIONAL
)
