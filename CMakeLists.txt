cmake_minimum_required(VERSION 3.10)
project(nitrate-lang)

enable_testing()

add_compile_options(-fsanitize=address -fsanitize=undefined)
add_link_options(-fsanitize=address -fsanitize=undefined)

add_compile_options(-g3)

# add_compile_options(-O3 -flto)
# add_link_options(-flto)

add_subdirectory(compiler)
