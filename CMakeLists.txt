cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)
option(COVERAGE "Enable coverage" OFF)
option(STRIP_OUTPUT "Strip output" OFF)
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
  enable_testing()
endif()

# Weird compiler warning with macro arguments. Disable it.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=gnu-zero-variadic-macro-arguments \
                                          -Wno-gnu-zero-variadic-macro-arguments       \
                                          -Wno-error=unused-command-line-argument      \
                                          -Wno-unused-command-line-argument"  )
else()
  message(FATAL_ERROR "Unsupported compiler pair: '${CMAKE_CXX_COMPILER}' and '${CMAKE_C_COMPILER}'")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Modern security hardening flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro,-z,now")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic -Werror \
                    -Wno-error=unused-parameter \
                    -Wno-error=missing-field-initializers   \
                    -Wno-missing-field-initializers \
                    -D__QUIX_IMPL__")

# Enable full optimization in general
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
  # Fix issue with ASAN and private aliases
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -asan-use-private-alias=1")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if(COVERAGE STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_subdirectory(libnitrate-core)
  add_subdirectory(libnitrate-lexer)
  add_subdirectory(libnitrate-seq)
  add_subdirectory(libnitrate-parser)
  add_subdirectory(libnitrate-ir)
  add_subdirectory(libnitrate-emit)
  add_subdirectory(libnitrate)
  add_subdirectory(qpkg)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================= CREATE RELEASE BUILD =================
  add_subdirectory(libnitrate-core)
  add_subdirectory(libnitrate-lexer)
  add_subdirectory(libnitrate-seq)
  add_subdirectory(libnitrate-parser)
  add_subdirectory(libnitrate-ir)
  add_subdirectory(libnitrate-emit)
  add_subdirectory(libnitrate)
  add_subdirectory(qpkg)
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
    if(TARGET ${lib} AND TARGET ${lib}::shared)
      target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    endif()
  endforeach()
endif()

if(BUILD_TESTING)
  foreach(lib libnitrate-basic-lexer-fuzz)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  endforeach()
endif()
