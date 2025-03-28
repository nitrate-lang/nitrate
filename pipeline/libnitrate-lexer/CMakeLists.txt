cmake_minimum_required(VERSION 3.15)
project(libnitrate-lexer CXX)

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 0)
set(__TARGET_PATCH_VERSION 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_HASH=\\\"${__TARGET_COMMIT_HASH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_BRANCH=\\\"${__TARGET_COMMIT_BRANCH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_DATE=\\\"${__TARGET_COMMIT_DATE}\\\"")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-lexer ${CXX_SOURCES})
target_include_directories(nitrate-lexer PUBLIC src "include" ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include)
target_link_libraries(nitrate-lexer PUBLIC nitrate-core)
add_dependencies(nitrate-lexer nitrate-core)

install(TARGETS nitrate-lexer DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
