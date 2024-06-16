cmake_minimum_required(VERSION 3.15)
project(libexecinfo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(execinfo-static STATIC stacktraverse.c execinfo.c)
target_compile_options(execinfo-static PRIVATE -O2 -pipe -fno-strict-aliasing -fstack-protector -fno-optimize-sibling-calls -fno-omit-frame-pointer)
