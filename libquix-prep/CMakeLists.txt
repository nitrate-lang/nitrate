cmake_minimum_required(VERSION 3.15)
project(libquix-prep CXX)

add_custom_target(
  quixlua
  COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua CFLAGS='-fPIC'
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua
)

set(LIB_QUIXLUA ${CMAKE_CURRENT_SOURCE_DIR}/deps/lua/liblua.a)
message(STATUS "libquix-prep: Building with Lua: ${LIB_QUIXLUA}")


#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QPREP_ID "\"QPREP_${SHA256SUMS}\"")
message(STATUS "libquix-prep VersionId: ${QPREP_ID}")

#================== CONFIGURE BUILD ==================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== SET BUILD FLAGS ==================
SET(BUILD_FLAGS_COMMON -Wno-return-type-c-linkage)
SET(PROJECT_RELEASE_FLAGS ${BUILD_FLAGS_COMMON} -DQPREP_ID=${QPREP_ID})
SET(PROJECT_DEBUG_FLAGS   ${BUILD_FLAGS_COMMON} 
                                                -DQPREP_ID="undefined"
                                                
                                                )

#================== GET SOURCES ==================
file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

#================== CREATE BUILD ==================
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #================== CREATE STATICLIB DEBUG BUILD ==================
  add_library(quix-prep STATIC ${CXX_SOURCES})
  target_include_directories(quix-prep PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include 
                                                          ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-prep PRIVATE quix-core quix-lexer ${LIB_QUIXLUA} crypto)
  add_dependencies(quix-prep quix-core quix-lexer quixlua)

  #================== CREATE SHAREDLIB DEBUG BUILD ==================
  add_library(quix-prep-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-prep-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep-shared PRIVATE ${PROJECT_DEBUG_FLAGS})
  target_link_libraries(quix-prep-shared PRIVATE quix-core quix-lexer ${LIB_QUIXLUA} crypto)
  add_dependencies(quix-prep-shared quix-core quix-lexer quixlua)
  set_target_properties(quix-prep-shared PROPERTIES OUTPUT_NAME quix-prep)

  add_subdirectory(tests)
  add_subdirectory(fuzzer)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE STATICLIB RELEASE BUILD ================
  add_library(quix-prep STATIC ${CXX_SOURCES})
  target_include_directories(quix-prep PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                          ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep PRIVATE ${PROJECT_RELEASE_FLAGS})
  target_link_options(quix-prep PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-prep PRIVATE quix-core quix-lexer ${LIB_QUIXLUA} crypto)
  add_dependencies(quix-prep quix-core quix-lexer quixlua)

  #================== CREATE SHAREDLIB RELEASE BUILD ================
  add_library(quix-prep-shared SHARED ${CXX_SOURCES})
  target_include_directories(quix-prep-shared PUBLIC src include ${CMAKE_SOURCE_DIR}/libquix-core/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                                                 ${CMAKE_SOURCE_DIR}/libquix-prep/deps)
  target_compile_options(quix-prep-shared PRIVATE ${PROJECT_RELEASE_FLAGS})
  target_link_options(quix-prep-shared PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  target_link_libraries(quix-prep-shared PRIVATE quix-core quix-lexer ${LIB_QUIXLUA} crypto)
  add_dependencies(quix-prep-shared quix-core quix-lexer quixlua)
  set_target_properties(quix-prep-shared PROPERTIES OUTPUT_NAME quix-prep)

  add_subdirectory(tests)
endif()

install(TARGETS quix-prep-shared DESTINATION lib)
install(TARGETS quix-prep DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
