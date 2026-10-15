# ============================================================================
# UniConv 性能基准测试构建配置 (Google Benchmark)
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Google Benchmark 依赖
# ----------------------------------------------------------------------------
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.1
    GIT_SHALLOW    TRUE
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

# ----------------------------------------------------------------------------
# 基准测试可执行文件
# ----------------------------------------------------------------------------
add_executable(UniConvBench
    bench_encoding.cpp
)

target_link_libraries(UniConvBench
    PRIVATE
        UniConv
        benchmark::benchmark
        benchmark::benchmark_main
)

target_include_directories(UniConvBench
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
   OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
   OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(UniConvBench PRIVATE /utf-8)
endif()

set_target_properties(UniConvBench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
