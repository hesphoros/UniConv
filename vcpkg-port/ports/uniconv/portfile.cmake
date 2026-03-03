vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO hesphoros/UniConv
    REF "v${VERSION}"
    SHA512 aa76aed82984765a842555ab770f6cfe3ddcf6cdd51bbc7745ae904b85596fce568bf5aadd65919703b7ded8d0ecead472649e0254f77bd9a13d5edfb9fcf1fd
    HEAD_REF main
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" UNICONV_BUILD_SHARED)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DUNICONV_BUILD_SHARED=${UNICONV_BUILD_SHARED}
        -DUNICONV_BUILD_TESTS=OFF
        -DUNICONV_LIBICONV_SOURCE=SYSTEM
        -DCMAKE_DISABLE_FIND_PACKAGE_Git=ON
        -DFETCHCONTENT_FULLY_DISCONNECTED=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME UniConv
    CONFIG_PATH lib/cmake/UniConv
)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
