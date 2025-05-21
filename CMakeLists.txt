cmake_minimum_required(VERSION 3.15)
project(nitrate-compiler-suite CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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
                    -Wno-error=unused-parameter                            \
                    -Wno-error=gnu -Wno-gnu                                \
                    -D__NITRATE_IMPL__")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Configure boost libraries
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_NO_EXCEPTIONS")

if(STRIP_OUTPUT)
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
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -DNITRATE_FLOWPTR_TRACE=1")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNITRATE_FLOWPTR_TRACE=0")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if(COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  execute_process(COMMAND git -C ${CMAKE_SOURCE_DIR} log --pretty=format:%ci -n1
    OUTPUT_VARIABLE __TARGET_COMMIT_DATE COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND date -u -d ${__TARGET_COMMIT_DATE} --iso-8601=seconds
    OUTPUT_VARIABLE __TARGET_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --short=16 HEAD
    OUTPUT_VARIABLE __TARGET_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND git -C ${CMAKE_SOURCE_DIR} rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE __TARGET_COMMIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
else()
  set(__TARGET_COMMIT_DATE "1970-01-01T00:00:00-00:00")
  set(__TARGET_COMMIT_HASH "0000000000000000")
  set(__TARGET_COMMIT_BRANCH "unknown")
endif()

add_subdirectory(pipeline)
add_subdirectory(test)
# add_subdirectory(no3)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  foreach(lib nitrate-core)
    target_compile_options(${lib} PRIVATE -fsanitize=address,undefined,fuzzer)
    target_link_options(${lib} PRIVATE -fsanitize=address,undefined,fuzzer)
  endforeach()
endif()

if(BUILD_TESTING)
  add_subdirectory(examples)
endif()
