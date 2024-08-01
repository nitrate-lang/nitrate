cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


file(GLOB_RECURSE QPKG_CXX_SOURCES "src/*.cc")

add_executable(qpkg ${QPKG_CXX_SOURCES})
target_include_directories(qpkg PRIVATE src )
target_compile_options(qpkg PRIVATE -Werror -O3 -g -Wall -Wextra -Wpedantic -Wno-error=overlength-strings -Wno-predefined-identifier-outside-function)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(qpkg PRIVATE  -DQUIX_BUILD_ID=${QUIX_BUILD_ID} -flto)
    target_link_libraries(qpkg PRIVATE yaml-cpp boost_filesystem)
    add_dependencies(qpkg quix-parser)
else()
    target_compile_options(qpkg PRIVATE  -DQUIX_BUILD_ID="undefined")
    target_link_libraries(qpkg PRIVATE yaml-cpp boost_filesystem -fsanitize=address -fsanitize=undefined )
    add_dependencies(qpkg quix-parser)
endif()

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
