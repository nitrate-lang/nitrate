cmake_minimum_required(VERSION 3.15)
project(libquix-parser-fuzzer CXX)

file(GLOB_RECURSE CXX_SOURCES "*.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(libquix-parser-fuzzer ${CXX_SOURCES})
target_include_directories(libquix-parser-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_compile_options(libquix-parser-fuzzer PRIVATE -O0)
target_link_libraries(libquix-parser-fuzzer PRIVATE quix-parser quix-lexer quix-prep quix-core)
add_dependencies(libquix-parser-fuzzer quix-parser quix-lexer quix-prep quix-core)
