cmake_minimum_required(VERSION 3.15)
project(libquix-lexer CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QLEX_ID "\"QLEX_${SHA256SUMS}\"")
message(STATUS "libquix-lexer VersionId: ${QLEX_ID}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic -Werror -Wno-error=unused-parameter)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DQLEX_ID=${QLEX_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O0 -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-error=unused-function -Wno-unused-parameter -DQLEX_ID="undefined")

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-lexer SHARED ${CXX_SOURCES})
  target_include_directories(quix-lexer PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-lexer PRIVATE quix-core)
  target_link_options(quix-lexer PRIVATE -fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-lexer SHARED ${CXX_SOURCES})
  target_include_directories(quix-lexer PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-lexer PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-lexer PRIVATE quix-core)
endif()

add_subdirectory(fuzzer)
add_subdirectory(tests)
