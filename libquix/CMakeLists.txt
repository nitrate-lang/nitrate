cmake_minimum_required(VERSION 3.15)
project(libquix CXX)

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
set(__TARGET_VERSION "\"QQUIX_${SHA256SUMS}\"")
message(STATUS "libquix VersionId: ${__TARGET_VERSION}")

#================== SET BUILD FLAGS ==================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"undefined\\\"")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(quix ${CXX_SOURCES})
target_include_directories(quix PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-prep/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-ir/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-codegen/include)
target_compile_options(quix PRIVATE ${PROJECT_DEBUG_FLAGS})
target_link_libraries(quix PUBLIC quix-core
  quix-lexer
  quix-prep
  quix-parser
  quix-ir
  quix-codegen)
add_dependencies(quix quix-core quix-lexer quix-prep quix-parser quix-ir quix-codegen)
set_target_properties(quix PROPERTIES OUTPUT_NAME quix)

if(BUILD_TESTING)
  add_subdirectory(test)
  add_subdirectory(fuzzer)
endif()

install(TARGETS quix DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
