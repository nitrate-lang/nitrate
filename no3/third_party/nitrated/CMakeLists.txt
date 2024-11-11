cmake_minimum_required(VERSION 3.15)
project(nitrated CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB_RECURSE SOURCES "src/*.cc")

find_library(libglog libglog.a REQUIRED)
find_library(libgflags libgflags.a REQUIRED)


add_library(nitrated_main STATIC ${SOURCES})
target_include_directories(nitrated_main PUBLIC src)
target_compile_options(nitrated_main PRIVATE -O3 -fno-exceptions -Wno-return-type-c-linkage)
target_link_libraries(nitrated_main PUBLIC nitrate ${libglog} ${libgflags} crypto)
add_dependencies(nitrated_main nitrate)
