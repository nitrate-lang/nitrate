cmake_minimum_required(VERSION 3.15)
project(libnitrate-emit CXX)

#================== CONFIGURE BUILD ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(__TARGET_VERSION "\"CODEGEN_${SHA256SUMS}\"")
message(STATUS "libnitrate-emit VersionId: ${__TARGET_VERSION}")

#================== GET LLVM LIBRARIES =====================
execute_process(COMMAND llvm-config-18 --ldflags
  OUTPUT_VARIABLE LLVM_LD_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND llvm-config-18 --link-static --libfiles
  OUTPUT_VARIABLE LLVM_LIBS_RAW
  OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS_RAW}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE "\n" " " LLVM_LIBS_BASH "${LLVM_LIBS_RAW}")

#################################################################

set(BUILD_FLAGS_COMMON
  "-Wno-deprecated-enum-enum-conversion  \
  -Wno-error=unused-but-set-variable     \
  -Wno-unused-parameter                  \
  -fno-exceptions                        \
                                         \
  -DTRANSCODE_TARGET_C11                 \
  -DTRANSCODE_TARGET_CXX11               \
  -DTRANSCODE_TARGET_TYPESCRIPT          \
  -DTRANSCODE_TARGET_RUST                \
  -DTRANSCODE_TARGET_PYTHON              \
  -DTRANSCODE_TARGET_CSHARP              \
  -DLLVM_SUUPORT_ALL_TARGETS")

find_library(libz z REQUIRED)
find_library(libzstd zstd REQUIRED)
find_library(libtinfo tinfo REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"undefined\\\" ${BUILD_FLAGS_COMMON}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\" ${BUILD_FLAGS_COMMON}")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-emit ${CXX_SOURCES})
target_include_directories(nitrate-emit PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-ir/include
  /usr/lib/llvm-18/include)
target_link_libraries(nitrate-emit PUBLIC nitrate-core nitrate-ir)
foreach(lib ${LLVM_LIBS})
  target_link_libraries(nitrate-emit PUBLIC ${lib})
  message(STATUS "Linking ${lib}")
endforeach()
target_link_libraries(nitrate-emit PUBLIC ${libz} ${libtinfo} ${libzstd})
add_dependencies(nitrate-emit nitrate-core nitrate-ir)
set_target_properties(nitrate-emit PROPERTIES OUTPUT_NAME nitrate-emit)

install(TARGETS nitrate-emit DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
