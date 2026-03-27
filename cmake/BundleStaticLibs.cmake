# BundleStaticLibs.cmake
# Merge multiple static libraries into a single archive (Linux/Unix).
# Uses ar MRI script for reliable cross-archive merging.
#
# Required variables (passed via -D):
#   TARGET_LIB     - Path to the output library (will be overwritten)
#   LIBS_TO_BUNDLE - Semicolon-separated list of library paths to merge into TARGET_LIB
#   CMAKE_AR       - Path to the ar archiver tool
# Optional:
#   CMAKE_RANLIB   - Path to ranlib (for index regeneration)

cmake_minimum_required(VERSION 3.16)

if(NOT TARGET_LIB)
    message(FATAL_ERROR "BundleStaticLibs: TARGET_LIB not specified")
endif()

if(NOT LIBS_TO_BUNDLE)
    message(STATUS "BundleStaticLibs: No libraries to bundle, skipping")
    return()
endif()

if(NOT CMAKE_AR)
    message(FATAL_ERROR "BundleStaticLibs: CMAKE_AR not specified")
endif()

# ar MRI's CREATE truncates the output file, so we must first rename the
# original library to a temporary path before adding it back.
set(_orig_lib "${TARGET_LIB}.orig")
file(RENAME "${TARGET_LIB}" "${_orig_lib}")

# Generate the MRI script
set(_mri_file "${TARGET_LIB}.mri")
file(WRITE  "${_mri_file}" "CREATE ${TARGET_LIB}\n")
file(APPEND "${_mri_file}" "ADDLIB ${_orig_lib}\n")
foreach(_lib IN LISTS LIBS_TO_BUNDLE)
    if(EXISTS "${_lib}")
        file(APPEND "${_mri_file}" "ADDLIB ${_lib}\n")
    else()
        message(WARNING "BundleStaticLibs: library not found, skipping: ${_lib}")
    endif()
endforeach()
file(APPEND "${_mri_file}" "SAVE\n")
file(APPEND "${_mri_file}" "END\n")

execute_process(
    COMMAND "${CMAKE_AR}" -M
    INPUT_FILE "${_mri_file}"
    RESULT_VARIABLE _result
    ERROR_VARIABLE  _error
)

if(NOT _result EQUAL 0)
    # Restore the original library on failure
    if(EXISTS "${_orig_lib}")
        file(RENAME "${_orig_lib}" "${TARGET_LIB}")
    endif()
    file(REMOVE "${_mri_file}")
    message(FATAL_ERROR "BundleStaticLibs: ar -M failed (exit ${_result}): ${_error}")
endif()

# Regenerate the archive index
if(CMAKE_RANLIB AND NOT "${CMAKE_RANLIB}" STREQUAL "")
    execute_process(COMMAND "${CMAKE_RANLIB}" "${TARGET_LIB}")
endif()

# Cleanup temporary files
file(REMOVE "${_orig_lib}")
file(REMOVE "${_mri_file}")

message(STATUS "BundleStaticLibs: merged into ${TARGET_LIB}")
