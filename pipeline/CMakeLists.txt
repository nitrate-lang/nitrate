cmake_minimum_required(VERSION 3.15)
project(nitrate-pipeline CXX)

add_subdirectory(libnitrate-core)
add_subdirectory(libnitrate-lexer)
add_subdirectory(libnitrate-seq)
add_subdirectory(libnitrate-parser)
# add_subdirectory(libnitrate-ir)
# add_subdirectory(libnitrate-emit)
add_subdirectory(libnitrate)
