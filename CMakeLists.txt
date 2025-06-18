cmake_minimum_required(VERSION 3.10)
project(nitrate-lang)

enable_testing()

add_compile_options(-O3 -flto)
add_link_options(-flto)

add_subdirectory(runtime)
add_subdirectory(compiler)
