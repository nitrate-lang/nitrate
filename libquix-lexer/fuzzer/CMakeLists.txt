cmake_minimum_required(VERSION 3.15)
project(libquix-lexer-fuzzer CXX)

set(SOURCE "fuzzer.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(FLAGS -O3 -g -Wall -Wextra -pedantic -Werror -Wno-gnu-zero-variadic-macro-arguments -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)

add_executable(libquix-lexer-fuzzer ${SOURCE})
target_include_directories(libquix-lexer-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_compile_options(libquix-lexer-fuzzer PRIVATE ${FLAGS})
target_link_libraries(libquix-lexer-fuzzer PRIVATE quix-lexer quix-core)
target_link_options(libquix-lexer-fuzzer PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
add_dependencies(libquix-lexer-fuzzer quix-lexer)
