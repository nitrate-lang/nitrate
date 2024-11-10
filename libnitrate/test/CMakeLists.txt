cmake_minimum_required(VERSION 3.14)
project(libnitrate-test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable for gtest
set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES "src/*.cc")

add_executable(libnitrate-test ${TEST_SOURCES})
target_link_libraries(libnitrate-test nitrate GTest::gtest_main)
target_include_directories(libnitrate-test PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_dependencies(libnitrate-test nitrate)

install(TARGETS libnitrate-test DESTINATION bin)

include(GoogleTest)
gtest_discover_tests(libnitrate-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
