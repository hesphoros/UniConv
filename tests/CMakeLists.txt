# ============================================================================
# UniConv 测试构建配置 (Google Test)
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# Google Test 依赖
# ----------------------------------------------------------------------------
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
)

# Windows: 防止 gtest 覆盖父项目的编译器/链接器设置
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# 启用 CTest
include(GoogleTest)

# ----------------------------------------------------------------------------
# 测试可执行文件
# ----------------------------------------------------------------------------
add_executable(UniConvTests
    test_basic.cpp
    test_encoding_conversion.cpp
    test_robustness.cpp
    test_integer_conversion.cpp
)

target_link_libraries(UniConvTests
    PRIVATE
        UniConv
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

target_include_directories(UniConvTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

set_target_properties(UniConvTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 自动发现并注册测试用例到 CTest
gtest_discover_tests(UniConvTests)
