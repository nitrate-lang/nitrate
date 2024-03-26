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


# Build benchmark
file(GLOB_RECURSE BENCHMARK_SOURCES "bench/*.cc")

add_executable(libquixcc-bench ${BENCHMARK_SOURCES})
target_include_directories(libquixcc-bench PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(libquixcc-bench PRIVATE -O3 -g -march=native -flto)
target_link_libraries(libquixcc-bench libquixcc)
target_link_options(libquixcc-bench PRIVATE -flto)
