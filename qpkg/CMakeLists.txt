cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++20 source files
file(GLOB_RECURSE QUIXCC_CXX_SOURCES "src/*.cc")

add_subdirectory(src/cache/blake3)

# Get LLVM flags from llvm-config-14 --cxxflags
execute_process(COMMAND llvm-config-14 --cxxflags
                OUTPUT_VARIABLE LLVM_CXX_FLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm-config-14 --ldflags 
execute_process(COMMAND llvm-config-14 --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm-config-14 --link-static --libfiles
execute_process(COMMAND llvm-config-14 --link-static --libfiles
OUTPUT_VARIABLE LLVM_LIBS
OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(qpkg ${QUIXCC_CXX_SOURCES})
target_include_directories(qpkg PRIVATE src ${CMAKE_SOURCE_DIR}/libquixcc/include)

if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(qpkg PRIVATE -g -Wall -Wextra -Wpedantic -Werror -O3 -flto)
    target_link_options(qpkg PRIVATE -flto)
    add_custom_command(TARGET qpkg POST_BUILD COMMAND strip $<TARGET_FILE:qpkg>)
    add_custom_command(TARGET qpkg POST_BUILD COMMAND upx $<TARGET_FILE:qpkg>)
else()
    target_compile_options(qpkg PRIVATE -g -Wall -Wextra -Wpedantic -Werror ${LLVM_CXX_FLAGS})
    target_link_options(qpkg PRIVATE -fprofile-arcs -ftest-coverage)
endif()
target_link_directories(qpkg PRIVATE ${CMAKE_BINARY_DIR}/libquixcc)
target_link_libraries(qpkg PRIVATE yaml-cpp blake3 quixcc ${LLVM_LIBS} tinfo z)
target_link_options(qpkg PRIVATE ${LLVM_LD_FLAGS})
add_dependencies(qpkg blake3 libquixcc-static)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()