cmake_minimum_required(VERSION 3.16)

# ============================================================================
# 🔧 子项目检测 - 修复核心问题
# ============================================================================
# 检测是否作为子项目被包含
if(DEFINED PROJECT_NAME)
    set(UNICONV_IS_SUBPROJECT TRUE)
else()
    set(UNICONV_IS_SUBPROJECT FALSE)
endif()

project(UniConv
    VERSION 2.0.0.2
    DESCRIPTION "A c++ library for variable encoding conversion"
    LANGUAGES C CXX
)

# ============================================================================
# 基本设置 - 根据子项目状态调整
# ============================================================================

# 只有作为主项目时才设置全局C++标准
if(NOT UNICONV_IS_SUBPROJECT)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Build options - 子项目时默认关闭测试
if(NOT UNICONV_IS_SUBPROJECT)
    option(UNICONV_BUILD_TESTS "Build UniConv tests" OFF)
    option(UNICONV_BUILD_SHARED "Build shared library instead of static" OFF)
else()
    option(UNICONV_BUILD_TESTS "Build UniConv tests" OFF)
    option(UNICONV_BUILD_SHARED "Build shared library instead of static" OFF)
endif()

# Set default build type to Release if not specified (仅主项目)
if(NOT UNICONV_IS_SUBPROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ============================================================================
# 🔧 配置文件生成 - 修复路径问题
# ============================================================================

# 包含自定义模块路径
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# 配置跨平台config.h - 使用正确的路径变量
if(UNICONV_IS_SUBPROJECT)
    # 作为子项目时，配置文件生成到构建目录
    message(STATUS "UniConv: Building as subproject")
    set(UNICONV_CONFIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in")
    set(UNICONV_CONFIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
    
    # 确保构建目录存在
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/iconv")
    
    # 如果存在ConfigureConfig.cmake，使用它；否则创建简单配置
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigureConfig.cmake")
        # 临时设置变量以修复路径问题
        set(CMAKE_SOURCE_DIR_BACKUP ${CMAKE_SOURCE_DIR})
        set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
        include(cmake/ConfigureConfig.cmake)
        set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR_BACKUP})
    else()
        # 简单的配置文件生成
        if(EXISTS "${UNICONV_CONFIG_INPUT}")
            configure_file(
                "${UNICONV_CONFIG_INPUT}"
                "${UNICONV_CONFIG_OUTPUT}"
                @ONLY
            )
        endif()
    endif()
else()
    # 作为主项目时，使用原有逻辑
    message(STATUS "UniConv: Building as main project")
    include(cmake/ConfigureConfig.cmake)
endif()

# ============================================================================
# 编译器设置
# ============================================================================

# Configure compiler-specific settings
if(MSVC)
    # Set UTF-8 encoding for MSVC
    add_compile_options(/utf-8)
    # Disable specific MSVC warnings
    add_compile_options(/wd4996)  # Disable deprecated function warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# 源文件和头文件
# ============================================================================

# Collect all source files (UniConv + iconv)
set(UNICONV_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniConv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/iconv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/localcharset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/relocatable.c
)

# Collect UniConv header files
set(UNICONV_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UniConv.h
)

# ============================================================================
# 库目标创建
# ============================================================================

# Create UniConv library (static or shared based on option)
if(UNICONV_BUILD_SHARED)
    add_library(UniConv SHARED ${UNICONV_SOURCES})
    # Define UNICONV_DLL for shared library build
    target_compile_definitions(UniConv PRIVATE UNICONV_DLL)
    target_compile_definitions(UniConv INTERFACE UNICONV_DLL_IMPORT)
else()
    add_library(UniConv STATIC ${UNICONV_SOURCES})
endif()

# 🔧 添加别名目标 - 提供一致接口
add_library(UniConv::UniConv ALIAS UniConv)

# Set UniConv library properties
set_target_properties(UniConv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "UniConv"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 🔧 修复包含目录 - 使用现代CMake方式
target_include_directories(UniConv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>  # 用于生成的config.h
        $<INSTALL_INTERFACE:include/UniConv>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv
        ${CMAKE_CURRENT_BINARY_DIR}/include/iconv  # 用于生成的config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 🔧 如果ConfigureConfig.cmake设置了子项目相关变量，添加额外的包含路径
if(UNICONV_IS_SUBPROJECT_CONFIG AND UNICONV_GENERATED_CONFIG_DIR)
    target_include_directories(UniConv PUBLIC 
        $<BUILD_INTERFACE:${UNICONV_GENERATED_CONFIG_DIR}>
    )
    message(STATUS "UniConv: Added generated config directory: ${UNICONV_GENERATED_CONFIG_DIR}")
endif()

# 输出目录设置（仅主项目）
if(NOT UNICONV_IS_SUBPROJECT)
    if(UNICONV_BUILD_SHARED)
        set_target_properties(UniConv PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
    else()
        set_target_properties(UniConv PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        )
    endif()
endif()

# ============================================================================
# 编译定义
# ============================================================================

# Compiler definitions
target_compile_definitions(UniConv PRIVATE
    $<$<BOOL:${MSVC}>:_UNICODE>
    $<$<BOOL:${MSVC}>:UNICODE>
    # iconv specific definitions
    LIBDIR=""
    BUILDING_LIBICONV
    ENABLE_RELOCATABLE=1
    IN_LIBRARY
    INSTALLDIR=""
    NO_XMALLOC
    set_relocation_prefix=libiconv_set_relocation_prefix
    relocate=libiconv_relocate
    HAVE_CONFIG_H
)

# ============================================================================
# 测试构建（仅主项目或明确请求时）
# ============================================================================

if(UNICONV_BUILD_TESTS AND NOT UNICONV_IS_SUBPROJECT)
    # Check if AllTests.cpp exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/AllTests.cpp)
        # Create a simple common.h for tests
        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/common.h
"#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>

class LightLogWriteImpl {
private:
    std::ofstream logFile;
public:
    void SetLogsFileName(const std::string& filename) {
        logFile.open(filename, std::ios::app);
    }
    void WriteLog(const std::string& level, const std::string& message) {
        std::cout << \"[\" << level << \"] \" << message << std::endl;
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            logFile << \"[\" << std::put_time(std::localtime(&time_t), \"%Y-%m-%d %H:%M:%S\") 
                    << \"] [\" << level << \"] \" << message << std::endl;
            logFile.flush();
        }
    }
    ~LightLogWriteImpl() { if (logFile.is_open()) logFile.close(); }
};

inline LightLogWriteImpl& GetLogger() { static LightLogWriteImpl logger; return logger; }
#define glogger GetLogger()
#define LOGINFO(msg)  glogger.WriteLog(\"INFO\", msg)
#define LOGERROR(msg) glogger.WriteLog(\"ERROR\", msg)
#define LOGDEBUG(msg) glogger.WriteLog(\"DEBUG\", msg)
#define LOGWARN(msg)  glogger.WriteLog(\"WARN\", msg)
#define LOGFATAL(msg) glogger.WriteLog(\"FATAL\", msg)
#define LOGTRACE(msg) glogger.WriteLog(\"TRACE\", msg)
#define LOGOK(msg)    glogger.WriteLog(\"OK\", msg)
")
        
        add_executable(UniConvTests ${CMAKE_CURRENT_SOURCE_DIR}/test/AllTests.cpp)
        
        # Set executable output directory
        set_target_properties(UniConvTests PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        
        # Link with UniConv library
        target_link_libraries(UniConvTests PRIVATE UniConv::UniConv)
        
        # Set include directories for tests
        target_include_directories(UniConvTests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}  # For generated common.h
        )
        
        # Additional test executables for error handling validation
        
        # Simple test for enhanced methods
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/simple_test.cpp)
            add_executable(SimpleTest ${CMAKE_CURRENT_SOURCE_DIR}/test/simple_test.cpp)
            set_target_properties(SimpleTest PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            target_link_libraries(SimpleTest PRIVATE UniConv::UniConv)
            target_include_directories(SimpleTest PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_BINARY_DIR}
            )
        endif()
        
        # Error handling consistency test
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/error_test.cpp)
            add_executable(ErrorTest ${CMAKE_CURRENT_SOURCE_DIR}/test/error_test.cpp)
            set_target_properties(ErrorTest PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            target_link_libraries(ErrorTest PRIVATE UniConv::UniConv)
            target_include_directories(ErrorTest PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_BINARY_DIR}
            )
        endif()
        
        # Diagnosis test for error handling analysis
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test/diagnose_test.cpp)
            add_executable(DiagnoseTest ${CMAKE_CURRENT_SOURCE_DIR}/test/diagnose_test.cpp)
            set_target_properties(DiagnoseTest PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            target_link_libraries(DiagnoseTest PRIVATE UniConv::UniConv)
            target_include_directories(DiagnoseTest PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_BINARY_DIR}
            )
        endif()
        
        # Auto-copy DLL to test executable directory when building shared library
        if(UNICONV_BUILD_SHARED AND WIN32)
            add_custom_command(TARGET UniConvTests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:UniConv>
                    $<TARGET_FILE_DIR:UniConvTests>
                COMMENT "Copying UniConv DLL to test executable directory"
            )
        endif()
        
        # Auto-copy test data directory to test executable directory
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/testdata)
            add_custom_command(TARGET UniConvTests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/testdata
                    $<TARGET_FILE_DIR:UniConvTests>/testdata
                COMMENT "Copying test data directory to test executable directory"
            )
        endif()
        
        # Enable CTest and add test with correct working directory
        enable_testing()
        add_test(NAME UniConvTest
                 COMMAND UniConvTests
                 WORKING_DIRECTORY $<TARGET_FILE_DIR:UniConvTests>)
        set_tests_properties(UniConvTest PROPERTIES
            TIMEOUT 60
            PASS_REGULAR_EXPRESSION "All tests completed successfully"
        )
        
        # Windows specific settings for console application
        if(WIN32)
            set_target_properties(UniConvTests PROPERTIES
                WIN32_EXECUTABLE FALSE
            )
        endif()
        
        message(STATUS "UniConv tests target created: UniConvTests")
    else()
        message(WARNING "AllTests.cpp not found in test/ directory, skipping test target creation")
    endif()
endif()

# ============================================================================
# 安装规则（仅主项目）
# ============================================================================

if(NOT UNICONV_IS_SUBPROJECT)
    include(GNUInstallDirs)

    # Install libraries
    install(TARGETS UniConv
        EXPORT UniConvTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
    )

    # Install headers
    install(FILES ${UNICONV_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
    )

    # Install encodings.inc (required by UniConv.h)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/encodings.inc)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/encodings.inc
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
        )
    endif()

    # Install iconv.h to correct iconv/ subdirectory (UniConv.h includes "iconv/iconv.h")
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv/iconv.h)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv/iconv.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv/iconv
        )
    endif()

    # Install generated config.h
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv/iconv
        )
    endif()

    # Export targets for CMake
    install(EXPORT UniConvTargets
        FILE UniConvTargets.cmake
        NAMESPACE UniConv::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
    )

    # Create and install package configuration files
    include(CMakePackageConfigHelpers)

    # Generate the config file that includes the exports
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in")
        configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
            NO_SET_AND_CHECK_MACRO
            NO_CHECK_REQUIRED_COMPONENTS_MACRO
        )
    endif()

    # Generate the version file for the config file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
    )

    # Install the configuration files
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/UniConvConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
    )
endif()

# ============================================================================
# 配置摘要
# ============================================================================

# Print configuration summary
message(STATUS "")
message(STATUS "============================================")
message(STATUS "UniConv Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Is Subproject: ${UNICONV_IS_SUBPROJECT}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: 17 (target property)")
message(STATUS "  Build shared library: ${UNICONV_BUILD_SHARED}")
message(STATUS "  Build tests: ${UNICONV_BUILD_TESTS}")
if(NOT UNICONV_IS_SUBPROJECT)
    message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "============================================")
message(STATUS "")

# ============================================================================
# 清理目标 - 删除生成的配置文件
# ============================================================================

# 添加一个自定义目标来清理生成的配置文件
add_custom_target(clean-config
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h"
    COMMENT "Cleaning generated configuration files"
    VERBATIM
)

# 添加一个完全清理目标（包括配置文件）
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_BINARY_DIR}/include"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Deep cleaning: removing all build artifacts and generated files"
    VERBATIM
)