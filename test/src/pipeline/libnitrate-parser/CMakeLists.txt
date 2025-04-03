cmake_minimum_required(VERSION 3.14)
project(nitrate-test-pipeline-libnitrate-parser CXX)

##############################################################################################################################
## Generate Parser test cases for ASTReader
##############################################################################################################################

file(GLOB_RECURSE AST_READER_TEST_VECTORS ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/libnitrate-parser/tree)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Canonical.cc
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Littered.cc
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Minified.cc
  COMMAND ${CMAKE_SOURCE_DIR}/tools/ASTReader_CreateTests.py ../tree .
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/ASTReader_CreateTests.py
  DEPENDS ${AST_READER_TEST_VECTORS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/
  COMMENT "Generating ASTReader Tests"
  VERBATIM
)
add_custom_target(nitrate-test-pipeline-libnitrate-parser DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Canonical.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Littered.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/core/ASTReader_Minified.cc
)

##############################################################################################################################
