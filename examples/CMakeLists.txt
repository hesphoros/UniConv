cmake_minimum_required(VERSION 3.10)
project(HighPerformanceDemo)

set(CMAKE_CXX_STANDARD 17)

# 添加包含目录
include_directories(../include)
include_directories(../include/iconv)

# 添加可执行文件
add_executable(high_performance_demo simple_demo.cpp)

# 链接库
target_link_libraries(high_performance_demo ../../build/lib/Debug/UniConv)

# 设置输出目录
set_target_properties(high_performance_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)