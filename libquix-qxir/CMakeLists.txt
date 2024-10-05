cmake_minimum_required(VERSION 3.15)
project(libquix-qxir CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QXIR_ID "\"QXIR_${SHA256SUMS}\"")
message(STATUS "libquix-qxir VersionId: ${QXIR_ID}")

#================== CONFIGURE BUILD ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic -Werror -Wno-mismatched-tags -Wno-gnu-zero-variadic-macro-arguments)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DQXIR_ID=${QXIR_ID} -Wno-error=unused-parameter)
SET(PROJECT_DEBUG_FLAGS ${BUILD_FLAGS_COMMON} -O3 
                                              -Wno-error=unused-variable 
                                              -Wno-error=unused-parameter
                                              -Wno-error=unused-function 
                                              -DQXIR_ID="undefined"
                                              -mllvm 
                                              -asan-use-private-alias=1)

if (MEMORY_OVER_SPEED)
  SET(PROJECT_RELEASE_FLAGS ${PROJECT_RELEASE_FLAGS} -DMEMORY_OVER_SPEED=1)
  SET(PROJECT_DEBUG_FLAGS ${PROJECT_DEBUG_FLAGS} -DMEMORY_OVER_SPEED=1)
endif()

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-qxir STATIC ${CXX_SOURCES})
  target_include_directories(quix-qxir PUBLIC src include
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include)
  target_compile_options(quix-qxir PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-qxir PRIVATE quix-core quix-parser deflate crypto)
  add_dependencies(quix-qxir quix-core quix-parser)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-qxir-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-qxir-shared PUBLIC src include
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include)
  target_compile_options(quix-qxir-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-qxir-shared PRIVATE quix-core quix-parser deflate crypto)
  add_dependencies(quix-qxir-shared quix-core quix-parser)
  set_target_properties(quix-qxir-shared PROPERTIES OUTPUT_NAME quix-qxir)

  add_subdirectory(tests)
  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-qxir STATIC ${CXX_SOURCES})
  target_include_directories(quix-qxir PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include)
  target_compile_options(quix-qxir PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
  target_link_options(quix-qxir PRIVATE)
  target_link_libraries(quix-qxir PRIVATE quix-core quix-parser deflate crypto)
  add_dependencies(quix-qxir quix-core quix-parser)

  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-qxir-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-qxir-shared PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include)
  target_compile_options(quix-qxir-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-qxir-shared PRIVATE -flto)
  target_link_libraries(quix-qxir-shared PRIVATE quix-core-shared quix-parser-shared deflate crypto)
  add_dependencies(quix-qxir-shared quix-core-shared quix-parser-shared)
  set_target_properties(quix-qxir-shared PROPERTIES OUTPUT_NAME quix-qxir)
endif()

install(TARGETS quix-qxir-shared DESTINATION lib)
install(TARGETS quix-qxir DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
