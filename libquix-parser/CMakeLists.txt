cmake_minimum_required(VERSION 3.15)
project(libquix-parser CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QPARSE_ID "\"QPARSE_${SHA256SUMS}\"")
message(STATUS "libquix-parser VersionId: ${QPARSE_ID}")

#================== CONFIGURE BUILD ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g3 -fPIC -Wall -Wextra -pedantic -Werror -Wno-mismatched-tags -Wno-gnu-zero-variadic-macro-arguments)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DQPARSE_ID=${QPARSE_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O0 -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-error=unused-function -Wno-unused-parameter -DQPARSE_ID="undefined")

if (MEMORY_OVER_SPEED)
  SET(PROJECT_RELEASE_FLAGS ${PROJECT_RELEASE_FLAGS} -DMEMORY_OVER_SPEED=1)
  SET(PROJECT_DEBUG_FLAGS ${PROJECT_DEBUG_FLAGS} -DMEMORY_OVER_SPEED=1)
endif()

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-parser STATIC ${CXX_SOURCES})
  target_include_directories(quix-parser PUBLIC src include
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include)
  target_compile_options(quix-parser PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-parser PRIVATE quix-core quix-lexer libdeflate.a)
  target_link_options(quix-parser PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-parser quix-core quix-lexer)

  add_subdirectory(tests)
  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-parser STATIC ${CXX_SOURCES})
  target_include_directories(quix-parser PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include)
  target_compile_options(quix-parser PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-parser PRIVATE -flto)
  target_link_libraries(quix-parser PRIVATE quix-core quix-lexer libdeflate.a)
  add_dependencies(quix-parser quix-core quix-lexer)

  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-parser-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-parser-shared PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include)
  target_compile_options(quix-parser-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-parser-shared PRIVATE -flto)
  target_link_libraries(quix-parser-shared PRIVATE quix-core-shared quix-lexer-shared deflate)
  add_dependencies(quix-parser-shared quix-core-shared quix-lexer-shared)
  set_target_properties(quix-parser-shared PROPERTIES OUTPUT_NAME quix-parser)
endif()