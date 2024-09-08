cmake_minimum_required(VERSION 3.15)
project(libquix-prep CXX)

add_subdirectory(deps/lua)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QPREP_ID "\"QPREP_${SHA256SUMS}\"")
message(STATUS "libquix-prep VersionId: ${QPREP_ID}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -fPIC -Wall -Wextra -pedantic -Werror -Wno-mismatched-tags -Wno-gnu-zero-variadic-macro-arguments -Wno-return-type-c-linkage -Wno-return-type-c-linkage)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DQPREP_ID=${QPREP_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O3 -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-error=unused-function -Wno-unused-parameter -DQPREP_ID="undefined")

if (MEMORY_OVER_SPEED)
  SET(PROJECT_RELEASE_FLAGS ${PROJECT_RELEASE_FLAGS} -DMEMORY_OVER_SPEED=1)
  SET(PROJECT_DEBUG_FLAGS ${PROJECT_DEBUG_FLAGS} -DMEMORY_OVER_SPEED=1)
endif()

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-prep STATIC ${CXX_SOURCES})
  target_include_directories(quix-prep PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include 
                                                          ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-prep PRIVATE quix-core quix-lexer ${LIB_QUIXLUA})
  target_link_options(quix-prep PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-prep quix-core quix-lexer quixlua)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-prep-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-prep-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-prep-shared PRIVATE quix-core quix-lexer ${LIB_QUIXLUA})
  target_link_options(quix-prep-shared PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-prep-shared quix-core quix-lexer quixlua)
  set_target_properties(quix-prep-shared PROPERTIES OUTPUT_NAME quix-prep)

  add_subdirectory(tests)
  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-prep STATIC ${CXX_SOURCES})
  target_include_directories(quix-prep PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-prep PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-prep PRIVATE quix-core quix-lexer ${LIB_QUIXLUA})
  add_dependencies(quix-prep quix-core quix-lexer quixlua)

  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-prep-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-prep-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-prep-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-prep-shared PRIVATE quix-core quix-lexer ${LIB_QUIXLUA})
  add_dependencies(quix-prep-shared quix-core quix-lexer quixlua)
  set_target_properties(quix-prep-shared PROPERTIES OUTPUT_NAME quix-prep)

  add_subdirectory(tests)
endif()

