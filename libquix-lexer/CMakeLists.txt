cmake_minimum_required(VERSION 3.15)
project(libquix-lexer CXX)

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
set(__TARGET_VERSION "\"QLEX_${SHA256SUMS}\"")
message(STATUS "libquix-lexer VersionId: ${__TARGET_VERSION}")

#================== SET BUILD FLAGS ==================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"undefined\\\"")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage -D__TARGET_VERSION=\\\"${__TARGET_VERSION}\\\"")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map")
else()
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(quix-lexer STATIC ${CXX_SOURCES})
target_include_directories(quix-lexer PUBLIC src "include" ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include)
target_link_libraries(quix-lexer PUBLIC quix-core)
add_dependencies(quix-lexer quix-core)

add_library(quix-lexer-shared SHARED ${CXX_SOURCES})
target_include_directories(quix-lexer-shared PUBLIC src "include" ${CMAKE_CURRENT_SOURCE_DIR}/../libquix-core/include)
target_link_libraries(quix-lexer-shared PUBLIC quix-core-shared)
set_target_properties(quix-lexer-shared PROPERTIES OUTPUT_NAME quix-lexer)
add_dependencies(quix-lexer-shared quix-core-shared)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(TARGETS quix-lexer-shared DESTINATION lib)
install(TARGETS quix-lexer DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
