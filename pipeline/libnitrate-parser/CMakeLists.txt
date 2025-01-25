cmake_minimum_required(VERSION 3.15)
project(libnitrate-parser CXX)

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
set(__TARGET_VERSION "\"NPAR_${SHA256SUMS}\"")
message(STATUS "libnitrate-parser VersionId: ${__TARGET_VERSION}")

#================== SET BUILD FLAGS ==================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"undefined\\\"")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

#================== COMPILE PROTOBUF ==================
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.h
  COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src/core --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src/core SyntaxTree.proto
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.proto
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Compiling SyntaxTree.proto"
  VERBATIM
)

add_custom_target(compile_proto DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.h)

#================== FIND SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== BUILD TARGET ==================
add_library(nitrate-parser ${CXX_SOURCES})
target_include_directories(nitrate-parser PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include)
target_link_libraries(nitrate-parser PUBLIC nitrate-core nitrate-lexer protobuf)
add_dependencies(nitrate-parser nitrate-core nitrate-lexer compile_proto)

install(TARGETS nitrate-parser DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
