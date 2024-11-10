cmake_minimum_required(VERSION 3.15)
project(libnitrate-fuzzer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(libnitrate-basic-lexer-fuzz "libnitrate-basic-lexer-fuzz.cc")
target_include_directories(libnitrate-basic-lexer-fuzz PUBLIC ../include ${CMAKE_SOURCE_DIR}/libnitrate/include)
target_link_libraries(libnitrate-basic-lexer-fuzz PUBLIC nitrate)
add_dependencies(libnitrate-basic-lexer-fuzz nitrate)

install(TARGETS libnitrate-basic-lexer-fuzz DESTINATION bin)
