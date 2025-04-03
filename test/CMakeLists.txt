cmake_minimum_required(VERSION 3.14)
project(nitrate-test CXX)

add_subdirectory(src/pipeline/libnitrate-parser)

file(GLOB_RECURSE TEST_SOURCES "src/*.cc")

add_library(nitrate-testing SHARED ${TEST_SOURCES})
target_link_libraries(nitrate-testing PUBLIC
  nitrate
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  gtest
)

target_include_directories(nitrate-testing PRIVATE
  "src"
  "${CMAKE_SOURCE_DIR}/libnitrate-core/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-lexer/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-seq/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-parser/include"
)
add_dependencies(nitrate-testing
  nitrate
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  nitrate-test-pipeline-libnitrate-parser
)

add_executable(nitrate-test main.cc)
target_link_libraries(nitrate-test nitrate-testing)
add_dependencies(nitrate-test nitrate-testing)
add_test(NAME nitrate-test COMMAND nitrate-test)

install(TARGETS nitrate-testing DESTINATION lib)
install(TARGETS nitrate-test DESTINATION bin)
