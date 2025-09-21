# Configure config.h for cross-platform compatibility
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)

# Package information
set(PACKAGE_NAME "UniConv")
set(PACKAGE_VERSION "${PROJECT_VERSION}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "")
set(PACKAGE_URL "")
set(PACKAGE_TARNAME "uniconv")
set(PACKAGE "${PACKAGE_TARNAME}")
set(VERSION "${PROJECT_VERSION}")

# Platform detection and Windows-specific settings
if(WIN32 OR CYGWIN OR MINGW)
    set(WINDOWS_NATIVE 1)
    message(STATUS "Detected Windows platform, enabling Windows-specific configuration")
else()
    set(WINDOWS_NATIVE 0)
endif()

# Check for standard headers
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)
check_include_file("wchar.h" HAVE_WCHAR_H)
check_include_file("wctype.h" HAVE_WCTYPE_H)
check_include_file("langinfo.h" HAVE_LANGINFO_H)
check_include_file("locale.h" HAVE_LOCALE_H)
check_include_file("errno.h" HAVE_ERRNO_H)
check_include_file("sys/param.h" HAVE_SYS_PARAM_H)

# Check for functions
check_function_exists("setlocale" HAVE_SETLOCALE)
check_function_exists("memmove" HAVE_MEMMOVE)

# Check for langinfo CODESET
if(HAVE_LANGINFO_H)
    check_symbol_exists("CODESET" "langinfo.h" HAVE_LANGINFO_CODESET)
else()
    set(HAVE_LANGINFO_CODESET 0)
endif()

# Check for mbstate_t
check_type_size("mbstate_t" MBSTATE_T)
if(HAVE_MBSTATE_T)
    set(HAVE_MBSTATE_T 1)
else()
    set(HAVE_MBSTATE_T 0)    
endif()

# Check for wcrtomb
check_function_exists("wcrtomb" HAVE_WCRTOMB)

# Visibility attributes
if(WIN32)
    set(HAVE_VISIBILITY 0)
else()
    check_c_source_compiles("
        __attribute__ ((visibility (\"default\"))) int foo(void) { return 0; }
        int main() { return foo(); }
    " HAVE_VISIBILITY)
endif()

# Set ICONV_CONST based on compiler and platform
if(MSVC OR WIN32)
    set(ICONV_CONST "const")
else()
    # Test if iconv uses const
    check_c_source_compiles("
        #include <iconv.h>
        int main() {
            iconv_t cd = iconv_open(\"\", \"\");
            const char *in = \"\";
            char *out = 0;
            size_t inleft = 0, outleft = 0;
            iconv(cd, &in, &inleft, &out, &outleft);
            return 0;
        }
    " ICONV_SECOND_ARGUMENT_IS_CONST)
    
    if(ICONV_SECOND_ARGUMENT_IS_CONST)
        set(ICONV_CONST "const")
    else()
        set(ICONV_CONST "")
    endif()
endif()

# Test for words_bigendian
include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

# DLL export/import macros for Windows
if(WIN32)
    set(DLL_VARIABLE "__declspec(dllimport)")
    if(BUILD_SHARED_LIBS OR UNICONV_BUILD_SHARED)
        set(BUILDING_DLL 1)
        set(DLL_VARIABLE "__declspec(dllexport)")
    else()
        set(BUILDING_DLL 0)
    endif()
else()
    set(DLL_VARIABLE "")
    set(BUILDING_DLL 0)
endif()

# Set USE_MBSTATE_T
if(HAVE_MBSTATE_T AND HAVE_WCRTOMB)
    set(USE_MBSTATE_T 1)
else()
    set(USE_MBSTATE_T 0)
endif()

# Broken WCHAR_T detection
if(WIN32)
    set(BROKEN_WCHAR_H 0)
else()
    # Test for broken wchar.h
    if(HAVE_WCHAR_H)
        check_c_source_compiles("
            #include <wchar.h>
            int main() { return 0; }
        " WCHAR_H_WORKS)
        if(NOT WCHAR_H_WORKS)
            set(BROKEN_WCHAR_H 1)
        else()
            set(BROKEN_WCHAR_H 0)
        endif()
    else()
        set(BROKEN_WCHAR_H 0)
    endif()
endif()

# Enable relocatable installation
set(ENABLE_RELOCATABLE 1)
set(INSTALLPREFIX "${CMAKE_INSTALL_PREFIX}")

# Set relocatable variables  
if(ENABLE_RELOCATABLE)
    set(INSTALLDIR "")
    set(LOCALEDIR "locale")
else()
    set(INSTALLDIR "${CMAKE_INSTALL_PREFIX}")
    set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
endif()

# Additional platform-specific settings
if(WIN32)
    set(ICONV_CONST "const")
    set(HAVE_VISIBILITY 0)
    set(ENABLE_EXTRA 1)
    set(HAVE_WORKING_O_NOFOLLOW 0)
    set(HAVE_WORKING_O_NOATIME 0)
else()
    check_function_exists("readlink" HAVE_READLINK)
    set(ENABLE_EXTRA 1)
    set(HAVE_WORKING_O_NOFOLLOW 1)
    set(HAVE_WORKING_O_NOATIME 1)
endif()

# Set __STDC__ for compatibility
set(__STDC__ 1)

# Memory manager settings
set(USE_HEAP_FOR_TRANSLATION_TABLE 0)
set(USE_DOS 0)
set(USE_OSF1 0)
set(USE_AIX 0)
set(USE_ZOS 0)

# Extended encoding support
set(ENABLE_EXTRA 1)

# Set default values for undefined variables
if(NOT DEFINED EILSEQ)
    set(EILSEQ 84)
endif()

if(NOT DEFINED HAVE_WORKING_O_NOFOLLOW)
    set(HAVE_WORKING_O_NOFOLLOW 0)
endif()

if(NOT DEFINED HAVE_WORKING_O_NOATIME)
    set(HAVE_WORKING_O_NOATIME 0)
endif()

if(NOT DEFINED HAVE_READLINK)
    set(HAVE_READLINK 0)
endif()

if(NOT DEFINED DOUBLE_SLASH_IS_DISTINCT_ROOT)
    set(DOUBLE_SLASH_IS_DISTINCT_ROOT 0)
endif()

# ============================================================================
# 🔧 修复：正确处理子项目配置文件路径
# ============================================================================

# 检测是否作为子项目被包含
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # 作为主项目构建 - 生成的文件也应该放在构建目录中
    set(UNICONV_CONFIG_IS_SUBPROJECT FALSE)
    set(UNICONV_CONFIG_INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in")
    set(UNICONV_CONFIG_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
    message(STATUS "UniConv ConfigureConfig: Building as main project")
    
    # 确保构建目录存在
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/iconv")
else()
    # 作为子项目构建
    set(UNICONV_CONFIG_IS_SUBPROJECT TRUE)
    set(UNICONV_CONFIG_INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in")
    set(UNICONV_CONFIG_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/iconv/config.h")
    message(STATUS "UniConv ConfigureConfig: Building as subproject")
    
    # 确保构建目录存在
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/iconv")
endif()

# 生成配置文件
if(EXISTS "${UNICONV_CONFIG_INPUT_PATH}")
    configure_file(
        "${UNICONV_CONFIG_INPUT_PATH}"
        "${UNICONV_CONFIG_OUTPUT_PATH}"
        @ONLY
    )
    message(STATUS "UniConv ConfigureConfig: Generated config.h at ${UNICONV_CONFIG_OUTPUT_PATH}")
    
    # 🔧 设置全局变量，供主CMakeLists.txt使用
    # 不在这里调用target_include_directories，因为目标可能还未创建
    if(UNICONV_CONFIG_IS_SUBPROJECT)
        set(UNICONV_GENERATED_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/include" PARENT_SCOPE)
        set(UNICONV_IS_SUBPROJECT_CONFIG TRUE PARENT_SCOPE)
    endif()
else()
    message(FATAL_ERROR "UniConv ConfigureConfig: config.h.in not found at ${UNICONV_CONFIG_INPUT_PATH}")
endif()

message(STATUS "UniConv ConfigureConfig: Configuration completed with the following key settings:")
message(STATUS "  WINDOWS_NATIVE: ${WINDOWS_NATIVE}")
message(STATUS "  HAVE_STDINT_H: ${HAVE_STDINT_H}")
message(STATUS "  HAVE_WCHAR_H: ${HAVE_WCHAR_H}")
message(STATUS "  HAVE_LANGINFO_CODESET: ${HAVE_LANGINFO_CODESET}")
message(STATUS "  ICONV_CONST: ${ICONV_CONST}")
message(STATUS "  WORDS_BIGENDIAN: ${WORDS_BIGENDIAN}")
message(STATUS "  IS_SUBPROJECT: ${UNICONV_CONFIG_IS_SUBPROJECT}")