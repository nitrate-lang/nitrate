cmake_minimum_required(VERSION 3.15)
project(libquix-lexer-tests CXX)

file(GLOB_RECURSE PROGRAMS "*.cc")

foreach(PROGRAM ${PROGRAMS})
  get_filename_component(PROGRAM_NAME ${PROGRAM} NAME_WE)
  add_executable(prep-${PROGRAM_NAME} ${PROGRAM})
  target_link_libraries(prep-${PROGRAM_NAME} PRIVATE quix-prep-shared)
  target_include_directories(prep-${PROGRAM_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/libquix-lexer/include 
    ${CMAKE_SOURCE_DIR}/libquix-prep/include)
  add_dependencies(prep-${PROGRAM_NAME} quix-prep-shared)
endforeach()
