cmake_minimum_required(VERSION 3.14)
project(libnitrate-parser-benchmarks CXX)

file(GLOB_RECURSE BENCHMARKS "*.cc")

foreach(BENCHMARK ${BENCHMARKS})
  get_filename_component(BENCHMARK_NAME ${BENCHMARK} NAME_WE)
  add_executable(${BENCHMARK_NAME} ${BENCHMARK})
  target_link_libraries(${BENCHMARK_NAME} nitrate-parser)
  target_include_directories(${BENCHMARK_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
  add_dependencies(${BENCHMARK_NAME} nitrate-parser)
  install(TARGETS ${BENCHMARK_NAME} DESTINATION bin)
endforeach()
