cmake_minimum_required(VERSION 3.15)
project(libnitrate-beta-opt CXX)

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 0)
set(__TARGET_PATCH_VERSION 0)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_HASH=\\\"${__TARGET_COMMIT_HASH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_BRANCH=\\\"${__TARGET_COMMIT_BRANCH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_DATE=\\\"${__TARGET_COMMIT_DATE}\\\"")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-beta-opt ${CXX_SOURCES})
target_include_directories(nitrate-beta-opt PUBLIC src "include")
target_link_libraries(nitrate-beta-opt PUBLIC)

install(TARGETS nitrate-beta-opt DESTINATION lib)
install(DIRECTORY "include/" DESTINATION "include")
