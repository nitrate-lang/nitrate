cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite)

option(ONLY_BUILD_LIBQUIXCC "Only build libquixcc" OFF)

if (BUILD_TESTING)
enable_testing()
endif()

add_subdirectory(libquixcc)

if (NOT ONLY_BUILD_LIBQUIXCC)
  add_subdirectory(qcc)
  add_subdirectory(qld)
  add_subdirectory(qpkg)

  if (LIBQUIXCC_STATIC)
    add_subdirectory(tools)
  endif()
endif()