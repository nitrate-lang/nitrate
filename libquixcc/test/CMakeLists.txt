cmake_minimum_required(VERSION 3.15)
project(libquixcc-tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
    FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the libquixcc library unit tests
file(GLOB_RECURSE TEST_SOURCES "unit/*.cc")

enable_testing()

add_executable(libquixcc-tests ${TEST_SOURCES})
target_include_directories(libquixcc-tests PRIVATE  ${CMAKE_CURRENT_SOURCE_DIR}/../include "${CMAKE_CURRENT_SOURCE_DIR}/../internal")
target_link_directories(libquixcc-tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../../libquixcc)
target_compile_options(libquixcc-tests PRIVATE -Wall -Wextra -Werror)
target_link_libraries(libquixcc-tests j GTest::gtest_main)
add_dependencies(libquixcc-tests libquixcc)
add_test(NAME libquixcc-tests COMMAND libquixcc-tests)

include(GoogleTest)
gtest_discover_tests(libquixcc-tests)
