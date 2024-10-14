cmake_minimum_required(VERSION 3.15)
project(libquix CXX)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QUIX_ID "\"QQUIX_${SHA256SUMS}\"")
message(STATUS "libquix VersionId: ${QUIX_ID}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON  -g 
                        -fPIC 
                        -Wall 
                        -Wextra 
                        -pedantic 
                        -Werror
                        -D__QUIX_IMPL__)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -O3 -DQUIX_ID=${QUIX_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} -O3 
                                                -DQUIX_ID="undefined"
                                                -mllvm 
                                                -asan-use-private-alias=1)

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix STATIC ${CXX_SOURCES})
  target_include_directories(quix PUBLIC src include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-prep/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-qxir/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-codegen/include)
  target_compile_options(quix PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix PRIVATE quix-core 
                                     quix-lexer 
                                     quix-prep 
                                     quix-parser 
                                     quix-qxir 
                                     quix-codegen)
  add_dependencies(quix quix-core quix-lexer quix-prep quix-parser quix-qxir quix-codegen)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-shared PUBLIC src include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-prep/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-qxir/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-codegen/include)
  target_compile_options(quix-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-shared PRIVATE quix-core
                                            quix-lexer
                                            quix-prep
                                            quix-parser
                                            quix-qxir
                                            quix-codegen)
  add_dependencies(quix-shared quix-core quix-lexer quix-prep quix-parser quix-qxir quix-codegen)
  set_target_properties(quix-shared PROPERTIES OUTPUT_NAME quix)

  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix STATIC ${CXX_SOURCES})
  target_include_directories(quix PUBLIC src include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-prep/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-qxir/include
                                             ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-codegen/include)
  target_compile_options(quix PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
  target_link_options(quix PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix PRIVATE quix-core 
                                     quix-lexer 
                                     quix-prep 
                                     quix-parser 
                                     quix-qxir 
                                     quix-codegen)
  add_dependencies(quix quix-core quix-lexer quix-prep quix-parser quix-qxir quix-codegen)

  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-shared PUBLIC src include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-lexer/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-prep/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-parser/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-qxir/include
                                                ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-codegen/include)
  target_compile_options(quix-shared PRIVATE ${PROJECT_RELEASE_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden -flto)
  target_link_options(quix-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map -flto)
  target_link_libraries(quix-shared PRIVATE quix-core 
                                            quix-lexer 
                                            quix-prep 
                                            quix-parser 
                                            quix-qxir 
                                            quix-codegen
                                            )
  add_dependencies(quix-shared quix-core quix-lexer quix-prep quix-parser quix-qxir quix-codegen)
  set_target_properties(quix-shared PROPERTIES OUTPUT_NAME quix)
endif()

add_subdirectory(test)

install(TARGETS quix-shared DESTINATION lib)
install(TARGETS quix DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
