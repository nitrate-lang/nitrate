cmake_minimum_required(VERSION 3.14)
project(nitrate-api-examples CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE EXAMPLE_SOURCES "*.cc")

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} nitrate)
  target_include_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  add_dependencies(${EXAMPLE_NAME} nitrate)
  install(TARGETS ${EXAMPLE_NAME} DESTINATION bin)
endforeach()
