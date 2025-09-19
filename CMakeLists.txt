cmake_minimum_required(VERSION 3.16)
project(UniConv
    VERSION 2.0.0.1
    DESCRIPTION "A c++ library for variable encoding conversion"
    LANGUAGES C CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(UNICONV_BUILD_TESTS "Build UniConv tests" OFF)
option(UNICONV_BUILD_SHARED "Build shared library instead of static" OFF)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Configure compiler-specific settings
if(MSVC)
    # Set UTF-8 encoding for MSVC
    add_compile_options(/utf-8)
    # Disable specific MSVC warnings
    add_compile_options(/wd4996)  # Disable deprecated function warnings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/iconv)

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

# Create UniConv library (static or shared based on option)
if(UNICONV_BUILD_SHARED)
    add_library(UniConv SHARED ${UNICONV_SOURCES})
    set_target_properties(UniConv PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
    # Define UNICONV_DLL for shared library build
    target_compile_definitions(UniConv PRIVATE UNICONV_DLL)
    target_compile_definitions(UniConv INTERFACE UNICONV_DLL_IMPORT)
else()
    add_library(UniConv STATIC ${UNICONV_SOURCES})
    set_target_properties(UniConv PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
endif()

# Set UniConv library properties
set_target_properties(UniConv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "UniConv"
)

# No need to link separate libraries since everything is compiled together

# Set include directories for UniConv
target_include_directories(UniConv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv
)

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

# Optional: Build tests
if(UNICONV_BUILD_TESTS)
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
        target_link_libraries(UniConvTests PRIVATE UniConv)
        
        # Set include directories for tests
        target_include_directories(UniConvTests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}  # For generated common.h
        )
        
        # Additional test executables for error handling validation
        
        # Simple test for enhanced methods
        add_executable(SimpleTest ${CMAKE_CURRENT_SOURCE_DIR}/test/simple_test.cpp)
        set_target_properties(SimpleTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        target_link_libraries(SimpleTest PRIVATE UniConv)
        target_include_directories(SimpleTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        # Error handling consistency test
        add_executable(ErrorTest ${CMAKE_CURRENT_SOURCE_DIR}/test/error_test.cpp)
        set_target_properties(ErrorTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        target_link_libraries(ErrorTest PRIVATE UniConv)
        target_include_directories(ErrorTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}
        )
        
        # Diagnosis test for error handling analysis
        add_executable(DiagnoseTest ${CMAKE_CURRENT_SOURCE_DIR}/test/diagnose_test.cpp)
        set_target_properties(DiagnoseTest PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )
        target_link_libraries(DiagnoseTest PRIVATE UniConv)
        target_include_directories(DiagnoseTest PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_BINARY_DIR}
        )
        
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
        add_custom_command(TARGET UniConvTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/testdata
                $<TARGET_FILE_DIR:UniConvTests>/testdata
            COMMENT "Copying test data directory to test executable directory"
        )
        
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

# Installation rules
include(GNUInstallDirs)

# Install libraries
install(TARGETS UniConv
    EXPORT UniConvTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(FILES ${UNICONV_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
)

# Install encodings.inc (required by UniConv.h)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/encodings.inc
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
)

# Install iconv.h to UniConv directory (since UniConv.h includes "iconv.h")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv/iconv.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/UniConv
)

# Export targets for CMake
install(EXPORT UniConvTargets
    FILE UniConvTargets.cmake
    NAMESPACE UniConv::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
)

# Create and install package configuration files
include(CMakePackageConfigHelpers)

# Generate the config file that includes the exports
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/UniConvConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/UniConvConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UniConv
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

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

# Print configuration summary
message(STATUS "")
message(STATUS "UniConv Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build shared library: ${UNICONV_BUILD_SHARED}")
message(STATUS "  Build tests: ${UNICONV_BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")