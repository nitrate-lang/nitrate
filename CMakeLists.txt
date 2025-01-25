cmake_minimum_required(VERSION 3.15)
project(nitrate-compiler-suite CXX)

option(MEMORY_OVER_SPEED "Optimize for low memory usage over speed" OFF)
option(COVERAGE "Enable code coverage" OFF)
option(STRIP_OUTPUT "Strip symbols from output" OFF)
option(BUILD_TESTING "Build test programs" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

if(BUILD_TESTING)
  enable_testing()
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(FATAL_ERROR "Unsupported compiler: '${CMAKE_CXX_COMPILER_ID}'")
endif()

# Modern security hardening flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic -Werror \
                    -Wno-error=unused-parameter \
                    -D__NITRATE_IMPL__")

# https://stackoverflow.com/questions/21266380/is-the-gnu-zero-variadic-macro-arguments-safe-to-ignore
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=gnu-zero-variadic-macro-arguments \
                                        -Wno-gnu-zero-variadic-macro-arguments")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

if(STRIP_OUTPUT STREQUAL "ON")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif()

# Inject the optimization intent
if(MEMORY_OVER_SPEED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMEMORY_OVER_SPEED=1")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMEMORY_OVER_SPEED=0")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNITRATE_FLOWPTR_TRACE=1")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNITRATE_FLOWPTR_TRACE=0")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if(COVERAGE STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

add_subdirectory(pipeline)
add_subdirectory(no3)

if(BUILD_TESTING)
  add_subdirectory(examples)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND BUILD_SHARED_LIBS)
  foreach(lib nitrate-core
    nitrate-lexer
    nitrate-seq
    nitrate-parser
    nitrate-ir
    nitrate-emit
    nitrate)
    target_compile_options(${lib} PRIVATE -flto)
    target_link_options(${lib} PRIVATE -flto)
  endforeach()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  foreach(lib nitrate-core
    nitrate-lexer
    nitrate-seq
    nitrate-parser
    nitrate-ir
    nitrate-emit
    nitrate)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  endforeach()
endif()

