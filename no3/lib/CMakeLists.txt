cmake_minimum_required(VERSION 3.15)
project(libno3 CXX)

find_library(libyaml_cpp libyaml-cpp.a REQUIRED)
find_library(libgit2 git2 REQUIRED)
find_library(libboost_streams boost_iostreams REQUIRED)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=overlength-strings")

file(GLOB_RECURSE NO3_CXX_SOURCES "src/*.cc")

add_library(no3-tool ${NO3_CXX_SOURCES})
target_include_directories(no3-tool PUBLIC src
  ${CMAKE_SOURCE_DIR}/libnitrate-core/include
  ${CMAKE_SOURCE_DIR}/libnitrate-lexer/include
  ${CMAKE_SOURCE_DIR}/libnitrate-seq/include
  ${CMAKE_SOURCE_DIR}/libnitrate-parser/include
  ${CMAKE_SOURCE_DIR}/libnitrate-ir/include
  ${CMAKE_SOURCE_DIR}/libnitrate-emit/include
  ${CMAKE_SOURCE_DIR}/no3/lib/include)
target_link_libraries(no3-tool PUBLIC
  nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core
  ${libyaml_cpp}
  ${libgit2}
  unwind
  ${libboost_streams})
add_dependencies(no3-tool nitrate-emit
  nitrate-ir
  nitrate-parser
  nitrate-lexer
  nitrate-seq
  nitrate-core)

install(TARGETS no3-tool DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

