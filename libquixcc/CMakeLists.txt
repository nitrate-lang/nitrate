cmake_minimum_required(VERSION 3.15)
project(libquixcc CXX)

add_subdirectory(deps/libexecinfo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

execute_process(COMMAND llvm14-config --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND llvm14-config --link-static --libfiles
OUTPUT_VARIABLE LLVM_LIBS_RAW
OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS_RAW}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE "\n" " " LLVM_LIBS_BASH "${LLVM_LIBS_RAW}")

SET(BUILD_FLAGS_COMMON -g -O3 -Wall -Wextra -pedantic -Werror -Wno-error=unused-parameter -Wno-error=mismatched-new-delete -DLLVM_SUUPORT_ALL_TARGETS)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -DNDEBUG)
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-unused-parameter)

file(GLOB_RECURSE LIBQUIX_CXX_SOURCES "src/*.cc")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_library(quixcc SHARED ${LIBQUIX_CXX_SOURCES})
  target_include_directories(quixcc PUBLIC "internal" include deps/libexecinfo)
  target_compile_options(quixcc PRIVATE ${PROJECT_DEBUG_FLAGS} -fprofile-arcs -ftest-coverage --coverage)
  target_link_options(quixcc PRIVATE ${LLVM_LD_FLAGS} -fprofile-arcs -ftest-coverage --coverage)

  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quixcc PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()

  target_link_libraries(quixcc PRIVATE libboost_serialization.a boost_filesystem libcrypto.a execinfo-static libstdc++.a z tinfo -fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #========= CREATE SHARED LIBRARY RELEASE BUILD ==========
  add_library(quixcc SHARED ${LIBQUIX_CXX_SOURCES})
  target_include_directories(quixcc PUBLIC "internal" include deps/libexecinfo)
  target_compile_options(quixcc PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quixcc PRIVATE ${LLVM_LD_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  

  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quixcc PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()

  target_link_libraries(quixcc PRIVATE libboost_serialization.a libboost_filesystem.a libcrypto.a execinfo-static libstdc++.a libz.a tinfo)

  add_custom_command(TARGET quixcc POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:quixcc>)
endif()


