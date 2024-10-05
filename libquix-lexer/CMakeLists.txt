cmake_minimum_required(VERSION 3.15)
project(libquix-lexer CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)

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
SET(BUILD_FLAGS_COMMON -g -fPIC -Wall -Wextra -pedantic -Werror -Wno-mismatched-tags -Wno-gnu-zero-variadic-macro-arguments -Wno-return-type-c-linkage -Wno-return-type-c-linkage)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DQLEX_ID=${QLEX_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O3 
                                                -fsanitize=fuzzer 
                                                -fsanitize=address 
                                                -fsanitize=undefined 
                                                -Wno-error=unused-variable 
                                                -Wno-error=unused-function 
                                                -DQLEX_ID="undefined"
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
  add_library(quix-lexer STATIC ${CXX_SOURCES})
  target_include_directories(quix-lexer PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-lexer PRIVATE quix-core)
  target_link_options(quix-lexer PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-lexer quix-core)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-lexer-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-lexer-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-lexer-shared PRIVATE quix-core)
  target_link_options(quix-lexer-shared PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-lexer-shared quix-core)
  set_target_properties(quix-lexer-shared PROPERTIES OUTPUT_NAME quix-lexer)

  add_subdirectory(tests)
  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-lexer STATIC ${CXX_SOURCES})
  target_include_directories(quix-lexer PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
  target_link_options(quix-lexer PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-lexer PRIVATE quix-core)
  add_dependencies(quix-lexer quix-core)

  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-lexer-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-lexer-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include)
  target_compile_options(quix-lexer-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-lexer-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-lexer-shared PRIVATE quix-core)
  add_dependencies(quix-lexer-shared quix-core)
  set_target_properties(quix-lexer-shared PROPERTIES OUTPUT_NAME quix-lexer)

  add_subdirectory(tests)
endif()

install(TARGETS quix-lexer-shared DESTINATION lib)
install(TARGETS quix-lexer DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
