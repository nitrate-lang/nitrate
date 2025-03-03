cmake_minimum_required(VERSION 3.15)
project(libnitrate-lexer CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"QLEX_${SHA256SUMS}\"")
message(STATUS "libnitrate-lexer VersionId: ${__TARGET_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-lexer ${CXX_SOURCES})
target_include_directories(nitrate-lexer PUBLIC src "include" ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include)
target_link_libraries(nitrate-lexer PUBLIC nitrate-core)
add_dependencies(nitrate-lexer nitrate-core)

install(TARGETS nitrate-lexer DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
