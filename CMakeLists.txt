cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite CXX)

# Weird compiler warning with macro arguments. Disable it.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=gnu-zero-variadic-macro-arguments -Wno-gnu-zero-variadic-macro-arguments")

if (STRIP_OUTPUT STREQUAL "ON")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

if (BUILD_TESTING)
enable_testing()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

if (COVERAGE STREQUAL "ON")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-prep)
  add_subdirectory(libquix-parser)
  add_subdirectory(libquix-qxir)
  add_subdirectory(libquix-codegen)
  add_subdirectory(libquix)
  add_subdirectory(qpkg)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================= CREATE RELEASE BUILD =================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-prep)
  add_subdirectory(libquix-parser)
  add_subdirectory(libquix-qxir)
  add_subdirectory(libquix-codegen)
  add_subdirectory(libquix)
  add_subdirectory(qpkg)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  foreach(lib quix-core quix-lexer quix-prep quix-parser quix-qxir quix-codegen quix)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  endforeach()

  foreach(lib quix-core-shared quix-lexer-shared quix-prep-shared quix-parser-shared quix-qxir-shared quix-codegen-shared quix-shared)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    target_link_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  endforeach()

  foreach(lib libquix-basic-lexer-fuzz libquix-prep-fuzzer libquix-qxir-fuzzer)
    target_compile_options(${lib} PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
    target_link_options(${lib} PRIVATE -fsanitize=fuzzer)
  endforeach()
endif()
