cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++20 source files
file(GLOB_RECURSE QUIXCC_CXX_SOURCES "src/*.cc")

add_subdirectory(src/cache/blake3)

add_executable(qpkg ${QUIXCC_CXX_SOURCES})
target_include_directories(qpkg PRIVATE src)

if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(qpkg PRIVATE -g -Wall -Wextra -Wpedantic -Werror -O3 -flto)
    target_link_options(qpkg PRIVATE -flto)
    add_custom_command(TARGET qpkg POST_BUILD COMMAND strip $<TARGET_FILE:qpkg>)
else()
    target_compile_options(qpkg PRIVATE -g -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_libraries(qpkg PRIVATE -lstdc++fs libstdc++.a crypto yaml-cpp blake3)
add_dependencies(qpkg blake3)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()