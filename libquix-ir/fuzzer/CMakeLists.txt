cmake_minimum_required(VERSION 3.15)
project(libquix-ir-fuzzer CXX)

set(SOURCE "fuzzer.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(libquix-ir-fuzzer ${SOURCE})
target_include_directories(libquix-ir-fuzzer PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix-core/include)
target_link_libraries(libquix-ir-fuzzer PUBLIC quix-ir quix-lexer quix-prep quix-core)
add_dependencies(libquix-ir-fuzzer quix-ir quix-lexer quix-prep quix-core)
