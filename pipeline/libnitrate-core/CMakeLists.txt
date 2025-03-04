cmake_minimum_required(VERSION 3.15)
project(libnitrate-core CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"QCORE_${SHA256SUMS}\"")
message(STATUS "libnitrate-core VersionId: ${__TARGET_VERSION}")

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 1)
set(__TARGET_PATCH_VERSION 0)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-core ${CXX_SOURCES})
target_include_directories(nitrate-core PUBLIC src "include")
target_link_libraries(nitrate-core PUBLIC unwind)

install(TARGETS nitrate-core DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
