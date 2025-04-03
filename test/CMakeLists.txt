cmake_minimum_required(VERSION 3.14)
project(nitrate-test CXX)

file(GLOB_RECURSE AST_TEST_VECTORS ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/libnitrate-parser/tree)

set(PARSER_TEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/libnitrate-parser")

##############################################################################################################################
## Generate Parser test cases for ASTReader
##############################################################################################################################

add_custom_command(
  OUTPUT ${PARSER_TEST_SRC}/core/ASTReader_Canonical.gen.cc
  OUTPUT ${PARSER_TEST_SRC}/core/ASTReader_Littered.gen.cc
  OUTPUT ${PARSER_TEST_SRC}/core/ASTReader_Minified.gen.cc
  COMMAND ${CMAKE_SOURCE_DIR}/tools/ASTReader_CreateTests.py ../tree .
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/ASTReader_CreateTests.py
  DEPENDS ${AST_TEST_VECTORS}
  WORKING_DIRECTORY ${PARSER_TEST_SRC}/core
  COMMENT "Generating ASTReader Tests"
  VERBATIM
)

##############################################################################################################################
## Generate Parser test cases for ASTWriter
##############################################################################################################################

add_custom_command(
  OUTPUT ${PARSER_TEST_SRC}/core/ASTWriter_Canonical.gen.cc
  OUTPUT ${PARSER_TEST_SRC}/core/ASTWriter_Littered.gen.cc
  OUTPUT ${PARSER_TEST_SRC}/core/ASTWriter_Minified.gen.cc
  COMMAND ${CMAKE_SOURCE_DIR}/tools/ASTWriter_CreateTests.py ../tree .
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/ASTWriter_CreateTests.py
  DEPENDS ${AST_TEST_VECTORS}
  WORKING_DIRECTORY ${PARSER_TEST_SRC}/core
  COMMENT "Generating ASTWriter Tests"
)

##############################################################################################################################
## Generate Parser test cases for CodeWriter
##############################################################################################################################

add_custom_command(
  OUTPUT ${PARSER_TEST_SRC}/core/CodeWriter.gen.cc
  COMMAND ${CMAKE_SOURCE_DIR}/tools/CodeWriter_CreateTests.py ../tree .
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/CodeWriter_CreateTests.py
  DEPENDS ${AST_TEST_VECTORS}
  WORKING_DIRECTORY ${PARSER_TEST_SRC}/core
  COMMENT "Generating CodeWriter Tests"
)

##############################################################################################################################

set(TESTING_PARSER_GENERATED_SOURCES
  ${PARSER_TEST_SRC}/core/ASTReader_Canonical.gen.cc
  ${PARSER_TEST_SRC}/core/ASTReader_Littered.gen.cc
  ${PARSER_TEST_SRC}/core/ASTReader_Minified.gen.cc

  ${PARSER_TEST_SRC}/core/ASTWriter_Canonical.gen.cc
  ${PARSER_TEST_SRC}/core/ASTWriter_Littered.gen.cc
  ${PARSER_TEST_SRC}/core/ASTWriter_Minified.gen.cc

  ${PARSER_TEST_SRC}/core/CodeWriter.gen.cc
)

add_custom_target(
  testing-generated-sources
  DEPENDS ${TESTING_PARSER_GENERATED_SOURCES}
)

##############################################################################################################################


file(GLOB_RECURSE STATIC_SOURCES "src/*.cc")

# Remove the generated files from the test sources: .gen.cc
foreach(file ${STATIC_SOURCES})
  if(file MATCHES ".gen.cc")
    list(REMOVE_ITEM STATIC_SOURCES ${file})
  endif()
endforeach()

add_library(nitrate-testing SHARED ${STATIC_SOURCES} ${TESTING_PARSER_GENERATED_SOURCES})
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
)

add_dependencies(nitrate-testing
  nitrate
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  testing-generated-sources
)

add_executable(nitrate-test main.cc)
target_link_libraries(nitrate-test nitrate-testing)
add_dependencies(nitrate-test nitrate-testing)
add_test(NAME nitrate-test COMMAND nitrate-test)

install(TARGETS nitrate-testing DESTINATION lib)
install(TARGETS nitrate-test DESTINATION bin)
