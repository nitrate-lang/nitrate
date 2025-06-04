cmake_minimum_required(VERSION 3.10)
project(nitrate-lang)

enable_testing()

add_compile_options(-g3)

# add_compile_options(-O3 -flto -fvisibility=hidden -fvisibility-inlines-hidden)
# add_link_options(-flto)

add_subdirectory(compiler)
