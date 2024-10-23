cmake_minimum_required(VERSION 3.15)
project(libquix-qxir CXX)

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
set(__TARGET_VERSION "\"QXIR_${SHA256SUMS}\"")
message(STATUS "libquix-qxir VersionId: ${__TARGET_VERSION}")

#================== SET BUILD FLAGS ==================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"undefined\\\"")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(quix-qxir STATIC ${CXX_SOURCES})
target_include_directories(quix-qxir PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include)
target_link_libraries(quix-qxir PUBLIC quix-core quix-parser deflate crypto)
add_dependencies(quix-qxir quix-core quix-parser)

add_library(quix-qxir-shared SHARED ${CXX_SOURCES})
target_include_directories(quix-qxir-shared PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include)
target_link_libraries(quix-qxir-shared PUBLIC quix-core-shared quix-parser-shared deflate crypto)
set_target_properties(quix-qxir-shared PROPERTIES OUTPUT_NAME quix-qxir)
add_dependencies(quix-qxir-shared quix-core-shared quix-parser-shared)

if (BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(fuzzer)
endif()

install(TARGETS quix-qxir-shared DESTINATION lib)
install(TARGETS quix-qxir DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
