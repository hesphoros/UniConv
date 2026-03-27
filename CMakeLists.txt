cmake_minimum_required(VERSION 3.16)


# 检测是否作为子项目被包含
if(DEFINED PROJECT_NAME)
    set(UNICONV_IS_SUBPROJECT TRUE)
else()
    set(UNICONV_IS_SUBPROJECT FALSE)
endif()

project(UniConv
    VERSION 3.3.2
    DESCRIPTION "High-performance C++ character encoding conversion library"
    LANGUAGES C CXX
)

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
option(UNICONV_BUILD_TESTS "Build tests" OFF) # 测试构建选项，默认关闭
option(UNICONV_BUILD_BENCHMARKS "Build benchmarks" OFF) # 性能基准测试，默认关闭
option(UNICONV_BUILD_SHARED "Build shared library instead of static" OFF) # 共享库构建选项，默认关闭
option(UNICONV_NO_THREAD_LOCAL "Disable thread_local storage (may be required for some DLL builds)" OFF) # 线程局部存储选项，默认关闭
option(UNICONV_USE_SIMDUTF "Use simdutf for SIMD-accelerated UTF conversions (optional dependency)" OFF) # SIMD 加速选项，默认关闭
option(UNICONV_BUNDLE_DEPS "Bundle all dependency static libraries into a single UniConv library" OFF) # 依赖合并选项，默认关闭

# libiconv 来源选择：FETCHCONTENT（推荐，自动拉取 libiconv-native）或 SYSTEM（系统库）
set(UNICONV_LIBICONV_SOURCE "FETCHCONTENT" CACHE STRING "Source for libiconv: FETCHCONTENT or SYSTEM")
set_property(CACHE UNICONV_LIBICONV_SOURCE PROPERTY STRINGS "FETCHCONTENT" "SYSTEM")

# 向后兼容旧选项
if(DEFINED UNICONV_USE_BUNDLED_LIBICONV)
    if(UNICONV_USE_BUNDLED_LIBICONV)
        message(WARNING "UNICONV_USE_BUNDLED_LIBICONV=ON is no longer supported. Using FETCHCONTENT instead.")
        set(UNICONV_LIBICONV_SOURCE "FETCHCONTENT" CACHE STRING "" FORCE)
    else()
        set(UNICONV_LIBICONV_SOURCE "SYSTEM" CACHE STRING "" FORCE)
    endif()
endif()

# ============================================================================
# 外部依赖（FetchContent）
# ============================================================================

include(FetchContent)

# ----------------------------------------------------------------------------
# libiconv 依赖处理
# ----------------------------------------------------------------------------
if(UNICONV_LIBICONV_SOURCE STREQUAL "FETCHCONTENT")
    message(STATUS "UniConv: Using FetchContent to fetch libiconv-native 1.18")
    
    FetchContent_Declare(
        libiconv
        GIT_REPOSITORY https://github.com/hesphoros/libiconv-native.git
        GIT_TAG        main
        GIT_SHALLOW    TRUE
    )
    
    # libiconv 构建选项
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)  # 强制构建静态库，避免符号冲突
    set(ICONV_ENABLE_EXTRA ON CACHE BOOL "" FORCE)  # 启用额外功能（如 Windows 上的 UTF-8 文件系统支持）
    set(ICONV_EMBED_MODE ON CACHE BOOL   "" FORCE)  # 启用嵌入模式，避免与系统 libiconv 符号冲突
    
    FetchContent_MakeAvailable(libiconv)
    
    set(UNICONV_HAS_FETCHCONTENT_LIBICONV TRUE)
    message(STATUS "UniConv: libiconv-native fetched successfully")
    
elseif(UNICONV_LIBICONV_SOURCE STREQUAL "SYSTEM")
    message(STATUS "UniConv: Using system iconv library")
    
    # 尝试查找系统 iconv
    find_path(ICONV_INCLUDE_DIR iconv.h)
    find_library(ICONV_LIBRARY NAMES iconv libiconv)
    
    # macOS/BSD: iconv 在 libc 中，不需要额外库
    if(NOT ICONV_LIBRARY)
        include(CheckFunctionExists)
        check_function_exists(iconv ICONV_IN_LIBC)
        if(ICONV_IN_LIBC)
            set(ICONV_LIBRARY "")
            message(STATUS "UniConv: iconv found in libc")
        else()
            message(FATAL_ERROR "UNICONV_LIBICONV_SOURCE=SYSTEM but system iconv not found. Use FETCHCONTENT or install libiconv.")
        endif()
    else()
        message(STATUS "UniConv: Found system iconv library: ${ICONV_LIBRARY}")
    endif()
    
    if(ICONV_INCLUDE_DIR)
        message(STATUS "UniConv: Found system iconv include: ${ICONV_INCLUDE_DIR}")
    endif()
    
    set(UNICONV_HAS_SYSTEM_LIBICONV TRUE)
else()
    message(FATAL_ERROR "Invalid UNICONV_LIBICONV_SOURCE: ${UNICONV_LIBICONV_SOURCE}. Use FETCHCONTENT or SYSTEM.")
endif()

# ----------------------------------------------------------------------------
# simdutf 依赖（可选 SIMD 加速）
# ----------------------------------------------------------------------------
if(UNICONV_USE_SIMDUTF)
    message(STATUS "UniConv: SIMD acceleration enabled via simdutf")
    
    FetchContent_Declare(
        simdutf
        GIT_REPOSITORY https://github.com/simdutf/simdutf.git
        GIT_TAG        v7.7.1
        GIT_SHALLOW    TRUE
    )
    
    # simdutf 构建选项
    set(SIMDUTF_TESTS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_TOOLS OFF CACHE BOOL "" FORCE)
    set(SIMDUTF_ICONV OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(simdutf)
    
    message(STATUS "UniConv: simdutf fetched successfully")
else()
    message(STATUS "UniConv: SIMD acceleration disabled (use -DUNICONV_USE_SIMDUTF=ON to enable)")
endif()

# ----------------------------------------------------------------------------
# parallel-hashmap 依赖（高性能并发哈希表）
# ----------------------------------------------------------------------------
# 首先尝试查找系统安装的 parallel-hashmap（vcpkg 或系统包管理器）
find_path(PHMAP_INCLUDE_DIR "parallel_hashmap/phmap.h")

if(PHMAP_INCLUDE_DIR)
    message(STATUS "UniConv: Found system parallel-hashmap: ${PHMAP_INCLUDE_DIR}")
    set(UNICONV_HAS_SYSTEM_PHMAP TRUE)
else()
    message(STATUS "UniConv: System parallel-hashmap not found, using FetchContent")
    
    FetchContent_Declare(
        phmap
        GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
        GIT_TAG        v2.0.0
        GIT_SHALLOW    TRUE
    )
    
    # parallel-hashmap 构建选项
    set(PHMAP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(PHMAP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(phmap)
    
    set(PHMAP_INCLUDE_DIR "${phmap_SOURCE_DIR}")
    set(UNICONV_HAS_FETCHCONTENT_PHMAP TRUE)
    message(STATUS "UniConv: parallel-hashmap fetched to ${PHMAP_INCLUDE_DIR}")
endif()

# ============================================================================
# CMake 模块路径
# ============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(UNICONV_IS_SUBPROJECT)
    message(STATUS "UniConv: building as subproject")
else()
    message(STATUS "UniConv: building as top-level project")
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

set(UNICONV_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/UniConv/UniConv.h
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
        $<BUILD_INTERFACE:${PHMAP_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
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

# ============================================================================
# libiconv 集成配置
# ============================================================================

if(UNICONV_HAS_FETCHCONTENT_LIBICONV)
    if(UNICONV_BUNDLE_DEPS AND NOT UNICONV_BUILD_SHARED)
        # 合并模式：只确保编译顺序和头文件可用，不链接库（符号已合并进 UniConv）
        add_dependencies(UniConv iconv charset)
    else()
        # 标准模式：链接 libiconv-native 提供的目标
        target_link_libraries(UniConv PRIVATE iconv)
    endif()
    
    # libiconv-native 的头文件需要 PUBLIC 暴露给使用 UniConv 的项目
    if(libiconv_SOURCE_DIR)
        target_include_directories(UniConv PUBLIC
            $<BUILD_INTERFACE:${libiconv_BINARY_DIR}/include>
        )
    endif()
    
elseif(UNICONV_HAS_SYSTEM_LIBICONV)
    # SYSTEM 模式：使用系统 iconv
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

# simdutf 集成（可选 SIMD 加速）
if(UNICONV_USE_SIMDUTF)
    target_compile_definitions(UniConv PUBLIC UNICONV_HAS_SIMDUTF=1)
    if(UNICONV_BUNDLE_DEPS AND NOT UNICONV_BUILD_SHARED)
        # 合并模式：只继承头文件，库代码已合并进 UniConv
        target_include_directories(UniConv PUBLIC
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:simdutf::simdutf,INTERFACE_INCLUDE_DIRECTORIES>>)
    else()
        target_link_libraries(UniConv PUBLIC simdutf::simdutf)
    endif()
endif()

# ============================================================================
# 静态库合并（将依赖嵌入 UniConv，输出单一自包含库）
# ============================================================================
if(UNICONV_BUNDLE_DEPS)
    if(UNICONV_BUILD_SHARED)
        message(WARNING "UNICONV_BUNDLE_DEPS is only effective for static builds, ignoring.")
    else()
        # 收集需要合并的依赖目标
        set(_UNICONV_BUNDLE_TARGETS "")
        if(UNICONV_HAS_FETCHCONTENT_LIBICONV)
            list(APPEND _UNICONV_BUNDLE_TARGETS iconv charset)
        endif()
        if(UNICONV_USE_SIMDUTF)
            list(APPEND _UNICONV_BUNDLE_TARGETS simdutf)
        endif()

        if(_UNICONV_BUNDLE_TARGETS)
            # 构建各依赖库的文件路径生成器表达式
            set(_BUNDLE_LIB_PATHS "")
            foreach(_target ${_UNICONV_BUNDLE_TARGETS})
                list(APPEND _BUNDLE_LIB_PATHS $<TARGET_FILE:${_target}>)
            endforeach()

            if(MSVC)
                # Windows MSVC: lib.exe 可直接合并静态库
                add_custom_command(TARGET UniConv POST_BUILD
                    COMMAND ${CMAKE_AR} /NOLOGO
                        /OUT:$<TARGET_FILE:UniConv>
                        $<TARGET_FILE:UniConv>
                        ${_BUNDLE_LIB_PATHS}
                    COMMENT "Bundling dependencies into UniConv.lib (self-contained)"
                    VERBATIM
                )
            elseif(APPLE)
                # macOS: libtool 合并静态库
                add_custom_command(TARGET UniConv POST_BUILD
                    COMMAND libtool -static -o $<TARGET_FILE:UniConv>
                        $<TARGET_FILE:UniConv>
                        ${_BUNDLE_LIB_PATHS}
                    COMMENT "Bundling dependencies into libUniConv.a (self-contained)"
                    VERBATIM
                )
            else()
                # Linux/Unix: 使用 cmake -P 脚本通过 ar MRI 合并
                # 构建分号分隔的路径字符串（使用 $<SEMICOLON> 避免 CMake 列表解析问题）
                list(LENGTH _UNICONV_BUNDLE_TARGETS _bundle_count)
                set(_BUNDLE_GENEXPR "")
                set(_bundle_idx 0)
                foreach(_target ${_UNICONV_BUNDLE_TARGETS})
                    if(_bundle_idx GREATER 0)
                        string(APPEND _BUNDLE_GENEXPR "$<SEMICOLON>")
                    endif()
                    string(APPEND _BUNDLE_GENEXPR "$<TARGET_FILE:${_target}>")
                    math(EXPR _bundle_idx "${_bundle_idx} + 1")
                endforeach()

                add_custom_command(TARGET UniConv POST_BUILD
                    COMMAND ${CMAKE_COMMAND}
                        -DTARGET_LIB=$<TARGET_FILE:UniConv>
                        "-DLIBS_TO_BUNDLE=${_BUNDLE_GENEXPR}"
                        -DCMAKE_AR=${CMAKE_AR}
                        -DCMAKE_RANLIB=${CMAKE_RANLIB}
                        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BundleStaticLibs.cmake"
                    COMMENT "Bundling dependencies into libUniConv.a (self-contained)"
                    VERBATIM
                )
            endif()

            message(STATUS "UniConv: Dependency bundling enabled — single self-contained static library")
        endif()
    endif()
endif()

# ============================================================================
# 安装规则（仅主项目）
# ============================================================================

if(NOT UNICONV_IS_SUBPROJECT)
    include(GNUInstallDirs)

    # 设置默认安装路径为构建目录下的 install/ 目录
    # 仅当用户未显式指定 CMAKE_INSTALL_PREFIX 时使用默认值
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH "Default install path" FORCE)
        message(STATUS "Install prefix set to: ${CMAKE_INSTALL_PREFIX}")
    endif()

    # vcpkg 使用独立的安装前缀处理 debug/release，直接使用 lib 和 bin
    # 非 vcpkg 构建时，为多配置生成器添加配置后缀
    if(DEFINED VCPKG_TARGET_TRIPLET OR DEFINED ENV{VCPKG_TARGET_TRIPLET})
        # vcpkg 构建：使用标准目录结构
        set(UNICONV_INSTALL_LIBDIR "lib")
        set(UNICONV_INSTALL_BINDIR "bin")
    elseif(CMAKE_CONFIGURATION_TYPES)
        # 非 vcpkg 多配置生成器：Visual Studio, Xcode, Ninja Multi-Config
        set(UNICONV_INSTALL_LIBDIR "lib/$<CONFIG>")
        set(UNICONV_INSTALL_BINDIR "bin/$<CONFIG>")
    else()
        # 非 vcpkg 单配置生成器：Makefile, Ninja
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(UNICONV_INSTALL_CONFIG_SUFFIX "Debug")
        else()
            set(UNICONV_INSTALL_CONFIG_SUFFIX "Release")
        endif()
        set(UNICONV_INSTALL_LIBDIR "lib/${UNICONV_INSTALL_CONFIG_SUFFIX}")
        set(UNICONV_INSTALL_BINDIR "bin/${UNICONV_INSTALL_CONFIG_SUFFIX}")
    endif()

    # 定义安装目录结构
    set(UNICONV_INSTALL_INCLUDEDIR "include")
    set(UNICONV_INSTALL_CMAKEDIR "lib/cmake/UniConv")

    # 安装 UniConv 库
    install(TARGETS UniConv
        EXPORT UniConvTargets
        LIBRARY DESTINATION ${UNICONV_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${UNICONV_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${UNICONV_INSTALL_BINDIR}
        INCLUDES DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}
    )

    # 安装 simdutf 库（如果启用）
    if(UNICONV_USE_SIMDUTF)
        install(TARGETS simdutf
            LIBRARY DESTINATION ${UNICONV_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${UNICONV_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${UNICONV_INSTALL_BINDIR}
        )
    endif()

    # 安装主头文件 UniConv.h
    install(FILES ${UNICONV_HEADERS}
        DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}/UniConv
    )

    # 安装 encodings.inc
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/UniConv/encodings.inc)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/UniConv/encodings.inc
            DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}/UniConv
        )
    endif()

    # parallel_hashmap 现在作为外部依赖，不再 vendor 安装
    # 用户需要通过 vcpkg 或系统包管理器安装 parallel-hashmap

    # 安装 iconv 目录下的头文件
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv
            DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}/UniConv
            FILES_MATCHING 
                PATTERN "*.h"
                PATTERN "*.inc"
        )
    endif()

    # 安装生成的 iconv/config.h（构建时生成的配置文件）
    if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h"
            DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}/UniConv/iconv
        )
    endif()

    # 安装 simdutf 头文件（如果启用）
    if(UNICONV_USE_SIMDUTF)
        # simdutf 的头文件位于 FetchContent 下载的目录中
        if(EXISTS "${simdutf_SOURCE_DIR}/include/simdutf.h")
            install(FILES "${simdutf_SOURCE_DIR}/include/simdutf.h"
                DESTINATION ${UNICONV_INSTALL_INCLUDEDIR}
            )
        endif()
    endif()

    # 导出目标
    install(EXPORT UniConvTargets
        FILE UniConvTargets.cmake
        NAMESPACE UniConv::
        DESTINATION ${UNICONV_INSTALL_CMAKEDIR}
    )

    # CMake 包配置
    include(CMakePackageConfigHelpers)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in")
        configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake"
            INSTALL_DESTINATION ${UNICONV_INSTALL_CMAKEDIR}
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
        DESTINATION ${UNICONV_INSTALL_CMAKEDIR}
    )

    # 打印安装信息
    message(STATUS "Install configuration:")
    message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "  Build type: ${UNICONV_INSTALL_CONFIG_SUFFIX}")
    message(STATUS "  Libraries: ${UNICONV_INSTALL_LIBDIR}")
    message(STATUS "  Binaries: ${UNICONV_INSTALL_BINDIR}")
    message(STATUS "  Headers: ${UNICONV_INSTALL_INCLUDEDIR}")
endif()

# ============================================================================
# 测试（仅主项目且 UNICONV_BUILD_TESTS=ON）
# ============================================================================
if(UNICONV_BUILD_TESTS AND NOT UNICONV_IS_SUBPROJECT)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# 性能基准测试（仅主项目且 UNICONV_BUILD_BENCHMARKS=ON）
# ============================================================================
if(UNICONV_BUILD_BENCHMARKS AND NOT UNICONV_IS_SUBPROJECT)
    add_subdirectory(benchmarks)
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
message(STATUS "  libiconv source: ${UNICONV_LIBICONV_SOURCE}")
message(STATUS "  SIMD acceleration (simdutf): ${UNICONV_USE_SIMDUTF}")
message(STATUS "  Tests: ${UNICONV_BUILD_TESTS}")
message(STATUS "  Benchmarks: ${UNICONV_BUILD_BENCHMARKS}")
message(STATUS "  Bundle dependencies: ${UNICONV_BUNDLE_DEPS}")
if(NOT UNICONV_IS_SUBPROJECT)
    message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "============================================")
message(STATUS "")
