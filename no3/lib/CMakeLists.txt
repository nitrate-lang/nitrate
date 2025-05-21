cmake_minimum_required(VERSION 3.15)
project(libnitrate-tool CXX)

find_library(libyaml_cpp libyaml-cpp.a REQUIRED)
find_library(libgit2 git2 REQUIRED)
find_library(libcurlpp curlpp REQUIRED)
find_library(libboost_streams boost_iostreams REQUIRED)
find_library(libboost_filesystem boost_filesystem REQUIRED)
find_library(libboost_program_options boost_program_options REQUIRED)
find_library(libunwind unwind REQUIRED)
find_library(libdeflate deflate REQUIRED)

#================== FIND SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")
remove(CXX_SOURCES "${PROTOBUF_SOURCE_CC}")
set(CXX_SOURCES ${CXX_SOURCES} ${PROTOBUF_SOURCE_CC})

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -Wno-error=overlength-strings")

# boost::any requires RTTI to be enabled. boost::program_options requires boost::any
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti")

# we can't avoid using exceptions in this file
# because it invokes libcurlpp which uses exceptions
set_source_files_properties(src/core/static/SPDX-API.cc PROPERTIES COMPILE_FLAGS "-frtti -fexceptions")

add_library(nitrate-tool ${CXX_SOURCES})

target_include_directories(nitrate-tool PUBLIC
  "src"
  "${CMAKE_SOURCE_DIR}/libnitrate-core/include"
  "${CMAKE_SOURCE_DIR}/no3/lib/include"
)

target_link_libraries(nitrate-tool PUBLIC
  "nitrate-testing"
  "nitrate-core"
  "${libunwind}"
  "${libyaml_cpp}"
  "${libgit2}"
  "${libcurlpp}"
  "${libboost_streams}"
  "${libboost_filesystem}"
  "${libboost_program_options}"
  "${libdeflate}"
)

install(TARGETS nitrate-tool DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
