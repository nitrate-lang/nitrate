cmake_minimum_required(VERSION 3.15)
project(libquix-parser CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(LIBQAST_VERSION "\"QAST_${SHA256SUMS}\"")
message(STATUS "LibQAST VersionId: ${QAST_}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic -Werror -Wno-error=unused-parameter)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DLIBQAST_VERSION=${LIBQAST_VERSION})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O0 -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-unused-parameter -DLIBQAST_VERSION="undefined")

#================== GET SOURCES ==================
file(GLOB_RECURSE LIBQAST_CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-parser SHARED ${LIBQAST_CXX_SOURCES})
  target_include_directories(quix-parser PUBLIC src include)
  target_compile_options(quix-parser PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-parser PRIVATE )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-parser SHARED ${LIBQAST_CXX_SOURCES})
  target_include_directories(quix-parser PUBLIC src include)
  target_compile_options(quix-parser PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-parser PRIVATE ${LLVM_LD_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-parser PRIVATE )
endif()
