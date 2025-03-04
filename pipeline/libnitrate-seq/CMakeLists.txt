cmake_minimum_required(VERSION 3.15)
project(libnitrate-seq CXX)

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 0)
set(__TARGET_PATCH_VERSION 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_HASH=\\\"${__TARGET_COMMIT_HASH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_BRANCH=\\\"${__TARGET_COMMIT_BRANCH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_DATE=\\\"${__TARGET_COMMIT_DATE}\\\"")

#===================== BUILD LUA ======================
add_custom_target(
  nitratelua
  ALL
  COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua CFLAGS='-fPIC -DLUA_COMPAT_5_3'
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua
)
set(LIB_NITRATELUA ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua/liblua.a)
message(STATUS "libnitrate-seq: Building with Lua: ${LIB_NITRATELUA}")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-seq ${CXX_SOURCES})
target_include_directories(nitrate-seq PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-parser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-seq/third_party)
target_link_libraries(nitrate-seq PUBLIC nitrate-core nitrate-lexer nitrate-parser ${LIB_NITRATELUA})
add_dependencies(nitrate-seq nitrate-core nitrate-lexer nitrate-parser nitratelua)

install(TARGETS nitrate-seq DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
