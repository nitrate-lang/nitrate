cmake_minimum_required(VERSION 3.15)
project(libnitrate CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"QNITRATE_${SHA256SUMS}\"")
message(STATUS "libnitrate VersionId: ${__TARGET_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate ${CXX_SOURCES})
target_include_directories(nitrate PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-seq/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-parser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-ir/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-emit/include)
target_compile_options(nitrate PRIVATE ${PROJECT_DEBUG_FLAGS})
target_link_libraries(nitrate PUBLIC nitrate-core
  nitrate-lexer
  nitrate-seq
  nitrate-parser
  nitrate-ir
  nitrate-emit)
add_dependencies(nitrate nitrate-core nitrate-lexer nitrate-seq nitrate-parser nitrate-ir nitrate-emit)
set_target_properties(nitrate PROPERTIES OUTPUT_NAME nitrate)

install(TARGETS nitrate DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
