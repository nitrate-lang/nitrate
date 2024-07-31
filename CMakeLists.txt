cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite CXX)

if (BUILD_TESTING)
enable_testing()
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-parser)
  
  # add_subdirectory(qcc)
  # add_subdirectory(qld)
  add_subdirectory(qpkg)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================= CREATE RELEASE BUILD =================
  add_subdirectory(libquix-core)
  add_subdirectory(libquix-lexer)
  add_subdirectory(libquix-parser)

  # add_subdirectory(qcc)
  # add_subdirectory(qld)
  add_subdirectory(qpkg)
endif()
  