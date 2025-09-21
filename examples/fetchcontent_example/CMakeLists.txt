cmake_minimum_required(VERSION 3.14)
project(UniConvFetchContentExample VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 FetchContent 包含 UniConv
include(FetchContent)

# 方法 1: 从本地路径获取（开发测试用）
FetchContent_Declare(
    UniConv
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..  # 指向 UniConv 根目录
)

# 方法 2: 从 Git 仓库获取（实际使用）
# FetchContent_Declare(
#     UniConv
#     GIT_REPOSITORY https://github.com/your-username/UniConv.git
#     GIT_TAG        main  # 或指定版本标签
# )

# 设置 UniConv 选项（在 FetchContent_MakeAvailable 之前）
set(UNICONV_BUILD_TESTS OFF CACHE BOOL "Disable UniConv tests in subproject")
set(UNICONV_BUILD_SHARED OFF CACHE BOOL "Build UniConv as static library")

# 让 FetchContent 下载并配置 UniConv
FetchContent_MakeAvailable(UniConv)

# 创建我们的示例可执行文件
add_executable(fetchcontent_demo main.cpp)

# 链接 UniConv
target_link_libraries(fetchcontent_demo PRIVATE UniConv)

# 输出配置信息
message(STATUS "==========================================")
message(STATUS "FetchContent Example Configuration:")
message(STATUS "  UniConv included as subproject: YES")
message(STATUS "  UniConv build tests: ${UNICONV_BUILD_TESTS}")
message(STATUS "  UniConv build shared: ${UNICONV_BUILD_SHARED}")
message(STATUS "==========================================")