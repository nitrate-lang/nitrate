cmake_minimum_required(VERSION 3.15)
project(libnitrate-ir CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"IR_${SHA256SUMS}\"")
message(STATUS "libnitrate-ir VersionId: ${__TARGET_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-ir ${CXX_SOURCES})
target_include_directories(nitrate-ir PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-parser/include)
target_link_libraries(nitrate-ir PUBLIC nitrate-core nitrate-parser)
add_dependencies(nitrate-ir nitrate-core nitrate-parser)

install(TARGETS nitrate-ir DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
