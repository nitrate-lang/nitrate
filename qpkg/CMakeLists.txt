cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"QPKG_${SHA256SUMS}\"")
message(STATUS "qpkg VersionId: ${__TARGET_VERSION}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/quixd)

find_library(libglog libglog.a REQUIRED)
find_library(libgflags libgflags.a REQUIRED)

file(GLOB_RECURSE QPKG_CXX_SOURCES "src/*.cc")

#================== CREATE APPLICATION RELEASE BUILD ================
add_executable(qpkg ${QPKG_CXX_SOURCES})
target_include_directories(qpkg PUBLIC src ${CMAKE_SOURCE_DIR}/libnitrate-core/include
  ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
  ${CMAKE_SOURCE_DIR}/libquix-prep/include
  ${CMAKE_SOURCE_DIR}/libquix-parser/include
  ${CMAKE_SOURCE_DIR}/libquix-ir/include
  ${CMAKE_SOURCE_DIR}/libquix-codegen/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/quixd/include)
target_compile_options(qpkg PRIVATE
  -Wno-error=overlength-strings
  -D__TARGET_VERSION=${__TARGET_VERSION}
)
target_link_libraries(qpkg PUBLIC libboost_filesystem.a
  quix-codegen
  quix-ir
  quix-parser
  nitrate-lexer
  quix-prep
  nitrate-core
  yaml-cpp
  quixd_main)
add_dependencies(qpkg quix-codegen
  quix-ir
  quix-parser
  nitrate-lexer
  quix-prep
  nitrate-core
  yaml-cpp
  quixd_main)

install(TARGETS qpkg DESTINATION bin)
