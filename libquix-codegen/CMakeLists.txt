cmake_minimum_required(VERSION 3.15)
project(libquix-codegen CXX)

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
message(STATUS "libquix-codegen VersionId: ${__TARGET_VERSION}")

#================== GET LLVM LIBRARIES =====================
execute_process(COMMAND llvm-config-14 --ldflags
  OUTPUT_VARIABLE LLVM_LD_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND llvm-config-14 --link-static --libfiles
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
  -fno-rtti                              \
  -fno-exceptions                        \
                                         \
  -DTRANSCODE_TARGET_C11                 \
  -DTRANSCODE_TARGET_CXX11               \
  -DTRANSCODE_TARGET_TYPESCRIPT          \
  -DTRANSCODE_TARGET_RUST                \
  -DTRANSCODE_TARGET_PYTHON              \
  -DTRANSCODE_TARGET_CSHARP              \
  -DLLVM_SUUPORT_ALL_TARGETS")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"undefined\\\" ${BUILD_FLAGS_COMMON}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\" ${BUILD_FLAGS_COMMON}")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(quix-codegen ${CXX_SOURCES})
target_include_directories(quix-codegen PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-ir/include
  /usr/lib/llvm-14/include)
target_link_libraries(quix-codegen PUBLIC quix-core quix-qxir)
foreach(lib ${LLVM_LIBS})
  target_link_libraries(quix-codegen PUBLIC ${lib})
  message(STATUS "Linking ${lib}")
endforeach()
target_link_libraries(quix-codegen PUBLIC z tinfo)
add_dependencies(quix-codegen quix-core quix-qxir)
set_target_properties(quix-codegen PROPERTIES OUTPUT_NAME quix-codegen)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(TARGETS quix-codegen DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
