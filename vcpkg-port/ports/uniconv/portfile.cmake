vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO hesphoros/UniConv
    REF "v${VERSION}"
    SHA512 0  # TODO: Update SHA512 after creating v3.1.0 release
    HEAD_REF main
)

# Check if shared library feature is enabled
vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        shared      UNICONV_BUILD_SHARED
        tests       UNICONV_BUILD_TESTS
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DUNICONV_BUILD_TESTS=OFF
        -DUNICONV_LIBICONV_SOURCE=SYSTEM
        -DCMAKE_DISABLE_FIND_PACKAGE_Git=ON
    OPTIONS_RELEASE
        -DCMAKE_BUILD_TYPE=Release
    OPTIONS_DEBUG
        -DCMAKE_BUILD_TYPE=Debug
)

vcpkg_cmake_build()

# Install the library
vcpkg_cmake_install()

# Fix CMake targets export
vcpkg_cmake_config_fixup(
    PACKAGE_NAME UniConv
    CONFIG_PATH lib/cmake/UniConv
)

# Copy header files
file(INSTALL 
    "${SOURCE_PATH}/include/UniConv.h"
    DESTINATION "${CURRENT_PACKAGES_DIR}/include"
)

file(INSTALL 
    "${SOURCE_PATH}/include/encodings.inc"
    DESTINATION "${CURRENT_PACKAGES_DIR}/include"
)

# Remove duplicate files
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

# Create usage file
file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" [[
UniConv provides CMake targets:

    find_package(UniConv CONFIG REQUIRED)
    target_link_libraries(main PRIVATE UniConv::UniConv)

UniConv is a high-performance C++ character encoding conversion library.
For usage examples, see: https://github.com/hesphoros/UniConv
]])