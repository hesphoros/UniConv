cmake_minimum_required(VERSION 3.16)

# ============================================================================
# 项目定义
# ============================================================================

# 检测是否作为子项目被包含
if(DEFINED PROJECT_NAME)
    set(UNICONV_IS_SUBPROJECT TRUE)
else()
    set(UNICONV_IS_SUBPROJECT FALSE)
endif()

project(UniConv
    VERSION 3.0.0.1
    DESCRIPTION "High-performance C++ character encoding conversion library"
    LANGUAGES C CXX
)

# ============================================================================
# 构建选项
# ============================================================================

# C++ 标准设置（仅主项目）
if(NOT UNICONV_IS_SUBPROJECT)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    
    # 默认构建类型
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

# 构建选项
option(UNICONV_BUILD_TESTS "Build tests" OFF)
option(UNICONV_BUILD_SHARED "Build shared library instead of static" OFF)
option(UNICONV_NO_THREAD_LOCAL "Disable thread_local storage (may be required for some DLL builds)" OFF)
option(UNICONV_USE_BUNDLED_LIBICONV "Use bundled libiconv sources in repository (ON preserves current behavior)" ON)

# ============================================================================
# 配置文件生成
# ============================================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(UNICONV_IS_SUBPROJECT)
    message(STATUS "UniConv: building as subproject")
    set(UNICONV_CONFIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in")
    set(UNICONV_CONFIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
    
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/iconv")
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigureConfig.cmake")
        set(CMAKE_SOURCE_DIR_BACKUP ${CMAKE_SOURCE_DIR})
        set(CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
        include(cmake/ConfigureConfig.cmake)
        set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR_BACKUP})
    elseif(EXISTS "${UNICONV_CONFIG_INPUT}")
        configure_file("${UNICONV_CONFIG_INPUT}" "${UNICONV_CONFIG_OUTPUT}" @ONLY)
    endif()
else()
    message(STATUS "UniConv: building as top-level project")
    include(cmake/ConfigureConfig.cmake)
endif()

# ============================================================================
# 编译器设置
# ============================================================================

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/wd4996)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# 库目标
# ============================================================================

# 源文件
set(UNICONV_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UniConv.cpp
)

if(UNICONV_USE_BUNDLED_LIBICONV)
    list(APPEND UNICONV_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/iconv.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/localcharset.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/relocatable.c
    )
else()
    # 尝试查找系统提供的 iconv/libiconv
    find_path(ICONV_INCLUDE_DIR iconv.h)
    find_library(ICONV_LIBRARY NAMES iconv libiconv)

    if(NOT ICONV_LIBRARY)
        message(FATAL_ERROR "UNICONV_USE_BUNDLED_LIBICONV=OFF but system libiconv not found. Either install libiconv or enable UNICONV_USE_BUNDLED_LIBICONV.")
    endif()

    if(ICONV_INCLUDE_DIR)
        message(STATUS "Found system iconv include: ${ICONV_INCLUDE_DIR}")
    endif()
    message(STATUS "Found system iconv library: ${ICONV_LIBRARY}")

endif()

set(UNICONV_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UniConv.h
)

# 创建库目标
if(UNICONV_BUILD_SHARED)
    add_library(UniConv SHARED ${UNICONV_SOURCES})
    target_compile_definitions(UniConv PRIVATE UNICONV_DLL)
    target_compile_definitions(UniConv INTERFACE UNICONV_DLL_IMPORT)
else()
    add_library(UniConv STATIC ${UNICONV_SOURCES})
endif()

# 别名目标
add_library(UniConv::UniConv ALIAS UniConv)

# 库属性
set_target_properties(UniConv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "UniConv"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 包含目录
target_include_directories(UniConv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include/UniConv>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 输出目录（仅主项目）
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

# 编译定义
target_compile_definitions(UniConv PRIVATE
    $<$<BOOL:${MSVC}>:_UNICODE>
    $<$<BOOL:${MSVC}>:UNICODE>
    LIBDIR=""
)

# 如果使用捆绑的 libiconv，添加对应的编译定义和包含路径
if(UNICONV_USE_BUNDLED_LIBICONV)
    target_compile_definitions(UniConv PRIVATE
        BUILDING_LIBICONV
        ENABLE_RELOCATABLE=1
        IN_LIBRARY
        INSTALLDIR=""
        NO_XMALLOC
        set_relocation_prefix=libiconv_set_relocation_prefix
        relocate=libiconv_relocate
        HAVE_CONFIG_H
    )
    target_include_directories(UniConv PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv
        ${CMAKE_CURRENT_BINARY_DIR}/include/iconv
    )
else()
    # 使用系统 libiconv，链接库并添加包含目录（若找到）
    if(ICONV_INCLUDE_DIR)
        target_include_directories(UniConv PRIVATE ${ICONV_INCLUDE_DIR})
    endif()
    if(ICONV_LIBRARY)
        target_link_libraries(UniConv PRIVATE ${ICONV_LIBRARY})
    endif()
endif()

# 条件编译选项
if(UNICONV_BUILD_SHARED)
    target_compile_definitions(UniConv PUBLIC UNICONV_BUILD_SHARED=1)
endif()

if(UNICONV_NO_THREAD_LOCAL)
    target_compile_definitions(UniConv PUBLIC UNICONV_NO_THREAD_LOCAL=1)
endif()

# ============================================================================
# 安装规则（仅主项目）
# ============================================================================

if(NOT UNICONV_IS_SUBPROJECT)
    include(GNUInstallDirs)

    # 安装库
    install(TARGETS UniConv
        EXPORT UniConvTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
    )

    # 安装头文件
    install(FILES ${UNICONV_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
    )

    # 安装 encodings.inc
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/encodings.inc)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/encodings.inc
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
        )
    endif()

    # 安装 iconv.h
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv/iconv.h)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv/iconv.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv/iconv
        )
    endif()

    # 安装生成的 config.h
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv/iconv
        )
    endif()

    # 导出目标
    install(EXPORT UniConvTargets
        FILE UniConvTargets.cmake
        NAMESPACE UniConv::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
    )

    # CMake 包配置
    include(CMakePackageConfigHelpers)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in")
        configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake"
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
            NO_SET_AND_CHECK_MACRO
            NO_CHECK_REQUIRED_COMPONENTS_MACRO
        )
    endif()

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/UniConvConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
    )
endif()

# ============================================================================
# 示例程序（仅主项目）
# ============================================================================

if(NOT UNICONV_IS_SUBPROJECT)
    add_subdirectory(examples)
endif()

# ============================================================================
# 配置摘要
# ============================================================================

message(STATUS "")
message(STATUS "============================================")
message(STATUS "UniConv configuration summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Subproject: ${UNICONV_IS_SUBPROJECT}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: 17")
message(STATUS "  Shared library: ${UNICONV_BUILD_SHARED}")
message(STATUS "  Tests: ${UNICONV_BUILD_TESTS}")
if(NOT UNICONV_IS_SUBPROJECT)
    message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "============================================")
message(STATUS "")
