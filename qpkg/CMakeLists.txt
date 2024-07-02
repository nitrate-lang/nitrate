cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++20 source files
file(GLOB_RECURSE QPKG_CXX_SOURCES "src/*.cc")

add_subdirectory(src/cache/blake3)

add_executable(qpkg ${QPKG_CXX_SOURCES})
target_include_directories(qpkg PRIVATE src ${CMAKE_SOURCE_DIR}/libquixcc/include)
target_compile_options(qpkg PRIVATE  -Werror -O3 -flto -g -Wall -Wextra -Wpedantic -Wno-error=overlength-strings -Wno-predefined-identifier-outside-function -DQUIX_BUILD_ID=${QUIX_BUILD_ID})

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_libraries(qpkg PRIVATE quixcc libyaml-cpp.a blake3 libstdc++.a)
    add_dependencies(qpkg quixcc)
    add_custom_command(TARGET qpkg POST_BUILD COMMAND strip $<TARGET_FILE:qpkg>)
else()
    target_link_libraries(qpkg PRIVATE quixcc libyaml-cpp.a blake3 libstdc++.a -fsanitize=address -fsanitize=undefined )
    add_dependencies(qpkg quixcc)
endif()

if (BUILD_TESTING)
    target_link_options(qpkg PRIVATE -fprofile-arcs -ftest-coverage)
endif()

target_link_directories(qpkg PRIVATE ${CMAKE_BINARY_DIR}/libquixcc)
add_dependencies(qpkg blake3)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()