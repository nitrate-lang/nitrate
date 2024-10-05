cmake_minimum_required(VERSION 3.15)
project(libquix-lexer-tests CXX)

file(GLOB_RECURSE PROGRAMS "*.cc")

foreach(PROGRAM ${PROGRAMS})
  get_filename_component(PROGRAM_NAME ${PROGRAM} NAME_WE)
  add_executable(${PROGRAM_NAME} ${PROGRAM})
  target_link_libraries(${PROGRAM_NAME} quix-lexer-shared quix-core-shared)
  target_include_directories(${PROGRAM_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libquix-lexer/include)
  add_dependencies(${PROGRAM_NAME} quix-lexer-shared quix-core-shared)
endforeach()
