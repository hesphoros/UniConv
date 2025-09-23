cmake_minimum_required(VERSION 3.10)
project(HighPerformanceDemo)

set(CMAKE_CXX_STANDARD 17)

# 添加包含目录
include_directories(../include)
include_directories(../include/iconv)

# 添加可执行文件
add_executable(high_performance_demo simple_demo.cpp)
add_executable(stage1_demo stage1_demo.cpp)
add_executable(branch_prediction_demo branch_prediction_demo.cpp)
add_executable(lru_cache_demo lru_cache_demo.cpp)

# 链接库
target_link_libraries(high_performance_demo ../../build/lib/Release/UniConv)
target_link_libraries(stage1_demo ../../build/lib/Release/UniConv)
target_link_libraries(branch_prediction_demo ../../build/lib/Release/UniConv)
target_link_libraries(lru_cache_demo ../../build/lib/Release/UniConv)

# 设置输出目录
set_target_properties(high_performance_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(stage1_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(branch_prediction_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(lru_cache_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)