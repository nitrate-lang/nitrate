cmake_minimum_required(VERSION 3.15)
project(libno3 CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"NO3_${SHA256SUMS}\"")
message(STATUS "no3 VersionId: ${__TARGET_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

find_library(libyaml_cpp libyaml-cpp.a REQUIRED)
find_library(libgit2 git2 REQUIRED)
find_library(libboost_streams boost_iostreams REQUIRED)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\" -Wno-error=overlength-strings")

file(GLOB_RECURSE NO3_CXX_SOURCES "src/*.cc")

add_library(no3-tool ${NO3_CXX_SOURCES})
target_include_directories(no3-tool PUBLIC src
  ${CMAKE_SOURCE_DIR}/libnitrate-core/include
  ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
  ${CMAKE_SOURCE_DIR}/libnitrate-seq/include
  ${CMAKE_SOURCE_DIR}/libnitrate-parser/include
  ${CMAKE_SOURCE_DIR}/libnitrate-ir/include
  ${CMAKE_SOURCE_DIR}/libnitrate-emit/include
  ${CMAKE_SOURCE_DIR}/no3/lib/include)
target_link_libraries(no3-tool PUBLIC
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  ${libyaml_cpp}
  ${libgit2}
  unwind
  ${libboost_streams})
add_dependencies(no3-tool nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core)

install(TARGETS no3-tool DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

