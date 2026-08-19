cmake_minimum_required(VERSION 3.16)
project(ZhiFenDesign VERSION 3.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
set(SOURCES
    src/main.cpp
)

# 可执行目标
add_executable(ZhiFenDesign ${SOURCES})

target_include_directories(ZhiFenDesign PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

# 编译选项
if(MSVC)
    target_compile_options(ZhiFenDesign PRIVATE /W3 /utf-8)
else()
    target_compile_options(ZhiFenDesign PRIVATE -Wall -Wextra)
endif()

# 版本定义
target_compile_definitions(ZhiFenDesign PRIVATE
    ZF_VERSION_MAJOR=3
    ZF_VERSION_MINOR=1
    ZF_VERSION_PATCH=0
    ZF_VERSION_STRING="3.1.0-FINAL"
)

# 安装规则
install(TARGETS ZhiFenDesign
    RUNTIME DESTINATION bin
)
install(FILES README.md
    DESTINATION .
    OPTIONAL
)
