cmake_minimum_required(VERSION 3.15)
project(libquix-fuzzer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_executable(libquix-basic-lexer-fuzz "libquix-basic-lexer-fuzz.cc")
target_include_directories(libquix-basic-lexer-fuzz PUBLIC ../include ${CMAKE_SOURCE_DIR}/libquix/include)
target_link_libraries(libquix-basic-lexer-fuzz PUBLIC quix)
add_dependencies(libquix-basic-lexer-fuzz quix)

install(TARGETS libquix-basic-lexer-fuzz DESTINATION bin)
