cmake_minimum_required(VERSION 3.16)
project(UniConvInstalledConsumer LANGUAGES CXX)

# This project deliberately uses only an installed, relocated UniConv package.
find_package(UniConv CONFIG REQUIRED)
add_executable(uniconv_consumer main.cpp)
target_link_libraries(uniconv_consumer PRIVATE UniConv::UniConv)
if(MSVC)
    target_compile_options(uniconv_consumer PRIVATE /utf-8)
endif()
enable_testing()
add_test(NAME installed_package COMMAND uniconv_consumer)
