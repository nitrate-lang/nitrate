cmake_minimum_required(VERSION 3.15)
project(quixd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB_RECURSE SOURCES "src/*.cc")

find_library(libglog libglog.a REQUIRED)
find_library(libgflags libgflags.a REQUIRED)

message(STATUS "src include path: ${CMAKE_CURRENT_SOURCE_DIR}/src")
add_library(quixd_main STATIC ${SOURCES})
target_include_directories(quixd_main PUBLIC src)
target_compile_options(quixd_main PRIVATE -O3 -fno-exceptions -Wno-return-type-c-linkage)
target_link_libraries(quixd_main PUBLIC quix ${libglog} ${libgflags} crypto)
add_dependencies(quixd_main quix)
