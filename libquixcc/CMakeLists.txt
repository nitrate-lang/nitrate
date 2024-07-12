cmake_minimum_required(VERSION 3.15)
project(libquixcc CXX)

#================== CALCULATE BUILD ID ==================
set(ENV_CONCAT_DATA "")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    string(CONCAT ENV_CONCAT_DATA "${ENV_CONCAT_DATA}${_variableName}=${${_variableName}}:")
endforeach()
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.cc" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND echo ${ENV_CONCAT_DATA} ${SHA256SUMS} COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(LIBQUIX_VERSION "\"QUIXCC_${SHA256SUMS}\"")
message(STATUS "LibQuix VersionId: ${LIBQUIX_VERSION}")

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

SET(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic -Werror -Wno-error=unused-parameter -Wno-error=mismatched-new-delete -DLLVM_SUUPORT_ALL_TARGETS)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DLIBQUIX_VERSION=${LIBQUIX_VERSION} -Wno-unused-parameter)
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O0 -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-unused-parameter -DLIBQUIX_VERSION="undefined")

file(GLOB_RECURSE LIBQUIX_CXX_SOURCES "src/*.cc")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE DEBUG BUILD ==================
  add_library(quixcc SHARED ${LIBQUIX_CXX_SOURCES})
  target_include_directories(quixcc PUBLIC "internal" include)
  target_compile_options(quixcc PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_options(quixcc PRIVATE ${LLVM_LD_FLAGS} )

  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quixcc PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()

  target_link_libraries(quixcc PRIVATE libboost_serialization.a boost_filesystem libcrypto.a libstdc++.a z tinfo -fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #========= CREATE SHARED LIBRARY RELEASE BUILD ==========
  add_library(quixcc SHARED ${LIBQUIX_CXX_SOURCES})
  target_include_directories(quixcc PUBLIC "internal" include)
  target_compile_options(quixcc PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quixcc PRIVATE ${LLVM_LD_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  

  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quixcc PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()

  target_link_libraries(quixcc PRIVATE libboost_serialization.a libboost_filesystem.a libcrypto.a libstdc++.a libz.a tinfo)
endif()


