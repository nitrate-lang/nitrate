cmake_minimum_required(VERSION 3.15)
project(libquix-qxir-fuzzer CXX)

set(SOURCE "fuzzer.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(FLAGS -O3 -g -Wall -Wextra -pedantic -Werror -Wno-error=mismatched-tags -Wno-error=gnu-zero-variadic-macro-arguments -Wno-gnu-zero-variadic-macro-arguments -Wno-mismatched-tags)

add_executable(libquix-qxir-fuzzer ${SOURCE})
target_include_directories(libquix-qxir-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_compile_options(libquix-qxir-fuzzer PRIVATE ${FLAGS})
target_link_libraries(libquix-qxir-fuzzer PRIVATE quix-qxir quix-lexer quix-prep quix-core)
add_dependencies(libquix-qxir-fuzzer quix-qxir quix-lexer quix-prep quix-core)
