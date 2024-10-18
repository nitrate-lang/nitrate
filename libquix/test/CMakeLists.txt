cmake_minimum_required(VERSION 3.14)
project(libquix-test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "src/*.cc")
add_executable(libquix-test ${TEST_SOURCES})
target_compile_options(libquix-test PRIVATE -Wall -Wextra -Werror -O3 -g)
target_link_libraries(libquix-test gtest_main quix)
target_include_directories(libquix-test PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_dependencies(libquix-test quix)

install(TARGETS libquix-test DESTINATION bin)
