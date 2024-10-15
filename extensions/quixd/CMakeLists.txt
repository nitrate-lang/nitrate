cmake_minimum_required(VERSION 3.15)
project(quixd CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if (STRIP_OUTPUT STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

if (BUILD_TESTING)
enable_testing()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

if (COVERAGE STREQUAL "ON")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

file(GLOB_RECURSE SOURCES "src/*.cc")

add_executable(quixd ${SOURCES})
target_include_directories(quixd PRIVATE src)
target_compile_options(quixd PRIVATE -O3 -Wall -Wextra -Werror -pedantic)

install(TARGETS quixd DESTINATION bin)
