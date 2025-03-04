cmake_minimum_required(VERSION 3.15)
project(libnitrate-parser CXX)

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 0)
set(__TARGET_PATCH_VERSION 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_HASH=\\\"${__TARGET_COMMIT_HASH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_BRANCH=\\\"${__TARGET_COMMIT_BRANCH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_DATE=\\\"${__TARGET_COMMIT_DATE}\\\"")

#================== SET BUILD FLAGS ==================
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
remove(CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.cc")
set(CXX_SOURCES ${CXX_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/core/SyntaxTree.pb.cc)

#================== BUILD TARGET ==================
add_library(nitrate-parser ${CXX_SOURCES})
target_include_directories(nitrate-parser PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include)
target_link_libraries(nitrate-parser PUBLIC nitrate-core nitrate-lexer protobuf)
add_dependencies(nitrate-parser nitrate-core nitrate-lexer compile_proto)

install(TARGETS nitrate-parser DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
