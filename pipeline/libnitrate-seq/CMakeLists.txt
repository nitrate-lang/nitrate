cmake_minimum_required(VERSION 3.15)
project(libnitrate-seq CXX)

#================== CONFIGURE BUILD ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"QPREP_${SHA256SUMS}\"")
message(STATUS "libnitrate-seq VersionId: ${__TARGET_VERSION}")

#===================== BUILD LUA ======================
add_custom_target(
  nitratelua
  ALL
  COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua CFLAGS='-fPIC -DLUA_COMPAT_5_3'
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua
)
set(LIB_NITRATELUA ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua/liblua.a)
message(STATUS "libnitrate-seq: Building with Lua: ${LIB_NITRATELUA}")

#================== SET BUILD FLAGS ==================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"undefined\\\"")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-seq ${CXX_SOURCES})
target_include_directories(nitrate-seq PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-seq/third_party)
target_link_libraries(nitrate-seq PUBLIC nitrate-core nitrate-lexer ${LIB_NITRATELUA} crypto)
add_dependencies(nitrate-seq nitrate-core nitrate-lexer nitratelua)

install(TARGETS nitrate-seq DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
