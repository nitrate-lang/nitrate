cmake_minimum_required(VERSION 3.15)
project(libquix-codegen CXX)

option(MEMORY_OVER_SPEED "Optimize for memory over speed" OFF)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(CODEGEN_ID "\"CODEGEN_${SHA256SUMS}\"")
message(STATUS "libquix-codegen VersionId: ${CODEGEN_ID}")

#================== GET LLVM LIBRARIES =====================
execute_process(COMMAND llvm-config-14 --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND llvm-config-14 --link-shared --libfiles
OUTPUT_VARIABLE LLVM_LIBS_RAW
OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS_RAW}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE "\n" " " LLVM_LIBS_BASH "${LLVM_LIBS_RAW}")

find_library(CLANG_LIB clang-cpp14 /usr/lib/llvm-14/lib)

#================== CONFIGURE BUILD ==================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SUPPORT_TARGETS -DTRANSCODE_TARGET_C11 
                    -DTRANSCODE_TARGET_CXX11 
                    -DTRANSCODE_TARGET_TYPESCRIPT 
                    -DTRANSCODE_TARGET_RUST 
                    -DTRANSCODE_TARGET_PYTHON 
                    -DTRANSCODE_TARGET_CSHARP
                    -DLLVM_SUUPORT_ALL_TARGETS)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic 
                       -Werror -Wno-mismatched-tags 
                       -Wno-gnu-zero-variadic-macro-arguments 
                       ${SUPPORT_TARGETS}
                       -I/usr/lib/llvm-14/include 
                       -Wno-deprecated-enum-enum-conversion
                       
                       )
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DNDEBUG -DCODEGEN_ID=${CODEGEN_ID} -Wno-unused-parameter)
SET(PROJECT_DEBUG_FLAGS ${BUILD_FLAGS_COMMON} -O3 -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined -Wno-error=unused-variable -Wno-error=unused-function -Wno-unused-parameter -Wno-unused-variable -DCODEGEN_ID="undefined")

if (MEMORY_OVER_SPEED)
  SET(PROJECT_RELEASE_FLAGS ${PROJECT_RELEASE_FLAGS} -DMEMORY_OVER_SPEED=1)
  SET(PROJECT_DEBUG_FLAGS ${PROJECT_DEBUG_FLAGS} -DMEMORY_OVER_SPEED=1)
endif()

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-codegen STATIC ${CXX_SOURCES})
  target_include_directories(quix-codegen PUBLIC src include
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-qxir/include)
  target_compile_options(quix-codegen PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-codegen PRIVATE quix-core z tinfo ${CLANG_LIB})
  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quix-codegen PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()
  target_link_options(quix-codegen PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-codegen quix-core quix-qxir)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-codegen-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-codegen-shared PUBLIC src include
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-qxir/include)
  target_compile_options(quix-codegen-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-codegen-shared PRIVATE quix-core z tinfo ${CLANG_LIB})
  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quix-codegen-shared PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()
  target_link_options(quix-codegen-shared PRIVATE -fsanitize=fuzzer -fsanitize=address -fsanitize=undefined)
  add_dependencies(quix-codegen-shared quix-core quix-qxir)
  set_target_properties(quix-codegen-shared PROPERTIES OUTPUT_NAME quix-codegen)

  add_subdirectory(tests)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-codegen STATIC ${CXX_SOURCES})
  target_include_directories(quix-codegen PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-qxir/include)
  target_compile_options(quix-codegen PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-codegen PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-codegen PRIVATE quix-core z tinfo ${CLANG_LIB})
  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quix-codegen PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()
  add_dependencies(quix-codegen quix-core quix-qxir)

  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-codegen-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-codegen-shared PUBLIC src include 
    ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-qxir/include)
  target_compile_options(quix-codegen-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-codegen-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-codegen-shared PRIVATE quix-core-shared z tinfo ${CLANG_LIB})
  foreach(lib ${LLVM_LIBS})
    target_link_libraries(quix-codegen-shared PRIVATE ${lib})
    message(STATUS "Linking ${lib}")
  endforeach()
  add_dependencies(quix-codegen-shared quix-core-shared quix-qxir)
  set_target_properties(quix-codegen-shared PROPERTIES OUTPUT_NAME quix-codegen)
endif()

install(TARGETS quix-codegen-shared DESTINATION lib)
install(TARGETS quix-codegen DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
