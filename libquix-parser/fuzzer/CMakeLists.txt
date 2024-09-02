cmake_minimum_required(VERSION 3.15)
project(libquix-parser-fuzzer CXX)

set(SOURCE "fuzzer.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(FLAGS -O3 -g -Wall -Wextra -pedantic -Werror -Wno-error=mismatched-tags -Wno-error=gnu-zero-variadic-macro-arguments -Wno-gnu-zero-variadic-macro-arguments -Wno-mismatched-tags -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)

add_executable(libquix-parser-fuzzer ${SOURCE})
target_include_directories(libquix-parser-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_compile_options(libquix-parser-fuzzer PRIVATE ${FLAGS})
target_link_libraries(libquix-parser-fuzzer PRIVATE quix-parser quix-lexer quix-core)
target_link_options(libquix-parser-fuzzer PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
add_dependencies(libquix-parser-fuzzer quix-parser)
