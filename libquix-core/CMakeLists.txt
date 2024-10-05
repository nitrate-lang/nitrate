cmake_minimum_required(VERSION 3.15)
project(libquix-core CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QCORE_ID "\"QCORE_${SHA256SUMS}\"")
message(STATUS "libquix-core VersionId: ${QCORE_ID}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -fPIC -Wall -Wextra -pedantic -Werror -Wno-gnu-zero-variadic-macro-arguments -Wno-mismatched-tags)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DQCORE_ID=${QCORE_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O3 
                                                -fsanitize=address 
                                                -fsanitize=undefined 
                                                -Wno-error=unused-variable 
                                                -DQCORE_ID="undefined"
                                                -mllvm 
                                                -asan-use-private-alias=1)

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-core STATIC ${CXX_SOURCES})
  target_include_directories(quix-core PUBLIC src include)
  target_compile_options(quix-core PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-core PRIVATE )

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-core-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-core-shared PUBLIC src include)
  target_compile_options(quix-core-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-core-shared PRIVATE)
  set_target_properties(quix-core-shared PROPERTIES OUTPUT_NAME quix-core)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-core STATIC ${CXX_SOURCES})
  target_include_directories(quix-core PUBLIC src include)
  target_compile_options(quix-core PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
  target_link_options(quix-core PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-core PRIVATE )

  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-core-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-core-shared PUBLIC src include)
  target_compile_options(quix-core-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-core-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-core-shared PRIVATE)
  set_target_properties(quix-core-shared PROPERTIES OUTPUT_NAME quix-core)
endif()

install(TARGETS quix-core-shared DESTINATION lib)
install(TARGETS quix-core DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
