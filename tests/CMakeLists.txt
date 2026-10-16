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
    test_native_numeric.cpp
    test_robustness.cpp
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

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
   OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
   OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(UniConvTests PRIVATE /utf-8)
endif()

set_target_properties(UniConvTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 自动发现并注册测试用例到 CTest
gtest_discover_tests(UniConvTests)

# ----------------------------------------------------------------------------
# Public macro contract and mixed-standard tests
# ----------------------------------------------------------------------------
function(uniconv_configure_macro_contract_target target_name cxx_standard)
    target_link_libraries(${target_name} PRIVATE UniConv)
    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD ${cxx_standard}
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
       OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
       OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        target_compile_options(${target_name} PRIVATE /utf-8 /W4 /WX)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE -Wundef -Werror)
    endif()

    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

add_executable(UniConvMacroContractCpp17 test_macro_contract.cpp)
uniconv_configure_macro_contract_target(UniConvMacroContractCpp17 17)

add_executable(UniConvMacroContractCpp20 test_macro_contract.cpp)
uniconv_configure_macro_contract_target(UniConvMacroContractCpp20 20)

include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
   OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
   OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(UNICONV_NO_CHAR8_T_FLAG "/Zc:char8_t-")
else()
    set(UNICONV_NO_CHAR8_T_FLAG "-fno-char8_t")
endif()

check_cxx_compiler_flag("${UNICONV_NO_CHAR8_T_FLAG}" UNICONV_HAS_NO_CHAR8_T_FLAG)
if(UNICONV_HAS_NO_CHAR8_T_FLAG)
    add_executable(UniConvMacroContractCpp20NoChar8T test_macro_contract.cpp)
    target_compile_options(UniConvMacroContractCpp20NoChar8T PRIVATE ${UNICONV_NO_CHAR8_T_FLAG})
    uniconv_configure_macro_contract_target(UniConvMacroContractCpp20NoChar8T 20)
endif()

add_library(UniConvMixedCpp17 OBJECT test_mixed_standard_cpp17.cpp)
target_link_libraries(UniConvMixedCpp17 PRIVATE UniConv)
set_target_properties(UniConvMixedCpp17 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

add_library(UniConvMixedCpp20 OBJECT test_mixed_standard_cpp20.cpp)
target_link_libraries(UniConvMixedCpp20 PRIVATE UniConv)
set_target_properties(UniConvMixedCpp20 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"
   OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"
   OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(UniConvMixedCpp17 PRIVATE /utf-8)
    target_compile_options(UniConvMixedCpp20 PRIVATE /utf-8)
endif()

add_executable(UniConvMixedStandard
    test_mixed_standard_main.cpp
    $<TARGET_OBJECTS:UniConvMixedCpp17>
    $<TARGET_OBJECTS:UniConvMixedCpp20>
)
target_link_libraries(UniConvMixedStandard PRIVATE UniConv)
set_target_properties(UniConvMixedStandard PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
add_test(NAME UniConvMixedStandard COMMAND UniConvMixedStandard)

# ----------------------------------------------------------------------------
# Expected compile failures and forbidden-surface checks
# ----------------------------------------------------------------------------
set(UNICONV_COMPILE_TEST_INCLUDE_DIRS
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
    "${PHMAP_INCLUDE_DIR}"
)

if(libiconv_BINARY_DIR)
    list(APPEND UNICONV_COMPILE_TEST_INCLUDE_DIRS "${libiconv_BINARY_DIR}/include")
endif()
if(ICONV_INCLUDE_DIR)
    list(APPEND UNICONV_COMPILE_TEST_INCLUDE_DIRS "${ICONV_INCLUDE_DIR}")
endif()

string(REPLACE ";" "|" UNICONV_COMPILE_TEST_INCLUDE_DIRS_ENCODED
    "${UNICONV_COMPILE_TEST_INCLUDE_DIRS}"
)

function(uniconv_add_expected_compile_failure test_name source_file cxx_standard expected_diagnostic)
    add_test(
        NAME ${test_name}
        COMMAND ${CMAKE_COMMAND}
            "-DCXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}"
            "-DCXX_SIMULATE_ID=${CMAKE_CXX_SIMULATE_ID}"
            "-DCXX_FRONTEND_VARIANT=${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}"
            "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source_file}"
            "-DSTANDARD=${cxx_standard}"
            "-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/${test_name}"
            "-DINCLUDE_DIRS_ENCODED=${UNICONV_COMPILE_TEST_INCLUDE_DIRS_ENCODED}"
            "-DEXPECTED_DIAGNOSTIC=${expected_diagnostic}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check_expected_compile_failure.cmake"
    )
endfunction()

uniconv_add_expected_compile_failure(
    UniConvRejectsCxx14
    compile_fail_cxx14.cpp
    14
    "UniConv requires C++17 or later"
)
uniconv_add_expected_compile_failure(
    UniConvRejectsPredefinedCxxStandard
    compile_fail_predefined_cxx_standard.cpp
    17
    "UNICONV_CXX_STANDARD is defined by <UniConv/UniConv.h> and must not be predefined"
)
uniconv_add_expected_compile_failure(
    UniConvRejectsPredefinedChar8T
    compile_fail_predefined_char8_t.cpp
    17
    "UNICONV_HAS_CHAR8_T is defined by <UniConv/UniConv.h> and must not be predefined"
)

add_test(
    NAME UniConvMacroSurface
    COMMAND ${CMAKE_COMMAND}
        "-DHEADER=${PROJECT_SOURCE_DIR}/include/UniConv/UniConv.h"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/check_macro_surface.cmake"
)
