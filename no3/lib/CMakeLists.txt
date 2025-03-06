cmake_minimum_required(VERSION 3.15)
project(libnitrate-tool CXX)

find_library(libyaml_cpp libyaml-cpp.a REQUIRED)
find_library(libgit2 git2 REQUIRED)
find_library(libboost_streams boost_iostreams REQUIRED)
find_library(libcurlpp curlpp REQUIRED)
find_library(libboost_filesystem boost_filesystem REQUIRED)
find_library(libunwind unwind REQUIRED)

#================== COMPILE PROTOBUF ==================
set(PROTOBUF_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/core/PackageConfigFormat.proto)
set(PROTOBUF_SOURCE_CC ${CMAKE_CURRENT_SOURCE_DIR}/src/core/PackageConfigFormat.pb.cc)
set(PROTOBUF_SOURCE_H ${CMAKE_CURRENT_SOURCE_DIR}/src/core/PackageConfigFormat.pb.h)

add_custom_command(
  OUTPUT ${PROTOBUF_SOURCE_CC} ${PROTOBUF_SOURCE_H}
  COMMAND protoc --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src/core --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/src/core ${PROTOBUF_SOURCE}
  DEPENDS ${PROTOBUF_SOURCE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Compiling ${PROTOBUF_SOURCE}"
  VERBATIM
)
add_custom_target(nitrate-tool-package-compile-proto DEPENDS ${PROTOBUF_SOURCE_CC} ${PROTOBUF_SOURCE_H})

#================== FIND SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")
remove(CXX_SOURCES "${PROTOBUF_SOURCE_CC}")
set(CXX_SOURCES ${CXX_SOURCES} ${PROTOBUF_SOURCE_CC})

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -Wno-error=overlength-strings")
set_source_files_properties(src/core/SPDX-API.cc PROPERTIES COMPILE_FLAGS "-frtti -fexceptions")

add_library(nitrate-tool ${CXX_SOURCES})

target_include_directories(nitrate-tool PUBLIC
  "src"
  "${CMAKE_SOURCE_DIR}/libnitrate-core/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-lexer/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-seq/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-parser/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-ir/include"
  "${CMAKE_SOURCE_DIR}/libnitrate-emit/include"
  "${CMAKE_SOURCE_DIR}/no3/lib/include"
)

target_link_libraries(nitrate-tool PUBLIC
  "nitrate-testing"
  "nitrate-emit"
  "nitrate-ir"
  "nitrate-parser"
  "nitrate-lexer"
  "nitrate-seq"
  "nitrate-core"
  "${libunwind}"
  "${libyaml_cpp}"
  "${libgit2}"
  "${libboost_streams}"
  "${libcurlpp}"
  "${libboost_filesystem}"
)
add_dependencies(nitrate-tool
  nitrate-testing
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  nitrate-tool-package-compile-proto
)

install(TARGETS nitrate-tool DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
