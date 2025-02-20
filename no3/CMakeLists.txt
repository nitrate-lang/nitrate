cmake_minimum_required(VERSION 3.15)
project(no3 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"NO3_${SHA256SUMS}\"")
message(STATUS "no3 VersionId: ${__TARGET_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(__TARGET_VERSION "\"undefined\"")
endif()

find_library(libglog libglog.a REQUIRED)
find_library(libgflags libgflags.a REQUIRED)
find_library(libyaml_cpp libyaml-cpp.a REQUIRED)
find_library(libgit2 git2 REQUIRED)
find_library(libboost_streams boost_iostreams REQUIRED)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\" -Wno-error=overlength-strings")

file(GLOB_RECURSE NO3_CXX_SOURCES "src/*.cc")

#================== CREATE APPLICATION BUILD ================
add_executable(no3 ${NO3_CXX_SOURCES})
target_include_directories(no3 PUBLIC src ${CMAKE_SOURCE_DIR}/libnitrate-core/include
  ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
  ${CMAKE_SOURCE_DIR}/libnitrate-seq/include
  ${CMAKE_SOURCE_DIR}/libnitrate-parser/include
  ${CMAKE_SOURCE_DIR}/libnitrate-ir/include
  ${CMAKE_SOURCE_DIR}/libnitrate-emit/include)
target_link_libraries(no3 PUBLIC
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  ${libyaml_cpp}
  ${libglog}
  ${libgflags}
  ${libgit2}
  unwind
  ${libboost_streams})
target_compile_options(no3 PUBLIC -DNO3_MAIN)
add_dependencies(no3 nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core)
target_link_options(no3 PRIVATE -static-libgcc -static-libstdc++)

if(BUILD_SHARED_LIBS STREQUAL "OFF")

  #================== CREATE LIBRARY BUILD ================
  add_library(no3-cli ${NO3_CXX_SOURCES})
  target_include_directories(no3-cli PUBLIC src ${CMAKE_SOURCE_DIR}/libnitrate-core/include
    ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
    ${CMAKE_SOURCE_DIR}/libnitrate-seq/include
    ${CMAKE_SOURCE_DIR}/libnitrate-parser/include
    ${CMAKE_SOURCE_DIR}/libnitrate-ir/include
    ${CMAKE_SOURCE_DIR}/libnitrate-emit/include)
  target_link_libraries(no3-cli PUBLIC
    nitrate-emit
    nitrate-ir
    nitrate-parser
    nitrate-lexer
    nitrate-seq
    nitrate-core
    ${libyaml_cpp}
    ${libglog}
    ${libgflags}
    ${libgit2}
    unwind
    ${libboost_streams})
  add_dependencies(no3-cli nitrate-emit
    nitrate-ir
    nitrate-parser
    nitrate-lexer
    nitrate-seq
    nitrate-core)

  install(TARGETS no3-cli DESTINATION lib)

endif()

install(TARGETS no3 DESTINATION bin)

