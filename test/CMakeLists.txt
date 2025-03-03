cmake_minimum_required(VERSION 3.14)
project(nitrate-test CXX)

if(BUILD_TESTING)
  file(GLOB_RECURSE TEST_SOURCES "src/*.cc")
else()
  set(TEST_SOURCES "src/Run.cc")
endif()

add_library(nitrate-testing ${TEST_SOURCES})
target_link_libraries(nitrate-testing
  nitrate
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  gtest)
target_include_directories(nitrate-testing PRIVATE
  src
  ${CMAKE_SOURCE_DIR}/libnitrate-core/include
  ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
  ${CMAKE_SOURCE_DIR}/libnitrate-seq/include
  ${CMAKE_SOURCE_DIR}/libnitrate-parser/include
  ${CMAKE_SOURCE_DIR}/libnitrate-ir/include
  ${CMAKE_SOURCE_DIR}/libnitrate-emit/include)
add_dependencies(nitrate-testing
  nitrate
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core)

install(TARGETS nitrate-testing DESTINATION lib)
