cmake_minimum_required(VERSION 3.15)
project(quixd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

file(GLOB_RECURSE SOURCES "src/*.cc")

find_library(libglog libglog.a REQUIRED)
find_library(libgflags libgflags.a REQUIRED)

add_executable(quixd ${SOURCES})
target_include_directories(quixd PRIVATE src
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../../libquix-core/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../../libquix-lexer/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../../libquix-parser/include)
target_compile_options(quixd PRIVATE -O3 -fno-exceptions)
target_link_libraries(quixd PRIVATE ${libglog} ${libgflags} crypto)

install(TARGETS quixd DESTINATION bin)
