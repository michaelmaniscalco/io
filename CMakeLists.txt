cmake_minimum_required(VERSION 3.16.3)

project(io)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("*** Build type not set.  defaulting to Release")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        -g
        -O0
        -march=native
    )
else()
    add_compile_options(
        -O3
        -march=native
    )
endif()

option(IO_BUILD_DEMO "Build the CLI demo" OFF)

include(FetchContent)

FetchContent_Declare(
    common
    GIT_REPOSITORY https://github.com/michaelmaniscalco/common.git
    GIT_TAG master
    SOURCE_DIR        "${CMAKE_BINARY_DIR}/common-src"
    BINARY_DIR        "${CMAKE_BINARY_DIR}/common-build"
    INSTALL_DIR       "${CMAKE_BINARY_DIR}"
    INSTALL_COMMAND   ""
)
FetchContent_MakeAvailable(common)
FetchContent_GetProperties(common)


add_subdirectory(src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
