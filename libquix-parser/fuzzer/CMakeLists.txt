cmake_minimum_required(VERSION 3.15)
project(libquix-parser-fuzzer CXX)

file(GLOB_RECURSE CXX_SOURCES "*.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(libquix-parser-fuzzer ${CXX_SOURCES})
target_include_directories(libquix-parser-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_compile_options(libquix-parser-fuzzer PRIVATE -g -O0 -Wall -Wextra -pedantic -Wno-error=mismatched-tags -Wno-error=gnu-zero-variadic-macro-arguments -Wno-gnu-zero-variadic-macro-arguments -Wno-mismatched-tags -fsanitize=address -fsanitize=undefined)
target_link_libraries(libquix-parser-fuzzer PRIVATE quix-parser quix-lexer quix-prep quix-core)
target_link_options(libquix-parser-fuzzer PRIVATE -fsanitize=address -fsanitize=undefined)
add_dependencies(libquix-parser-fuzzer quix-parser quix-lexer quix-prep quix-core)
