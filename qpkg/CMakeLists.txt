cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QPKG_ID "\"QPKG_${SHA256SUMS}\"")
message(STATUS "qpkg VersionId: ${QPKG_ID}")

file(GLOB_RECURSE QPKG_CXX_SOURCES "src/*.cc")

add_executable(qpkg ${QPKG_CXX_SOURCES})
target_include_directories(qpkg PRIVATE src )
target_compile_options(qpkg PRIVATE -Werror -O3 -g -Wall -Wextra -Wpedantic -Wno-error=overlength-strings -Wno-predefined-identifier-outside-function)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(qpkg PRIVATE  -DQPKG_ID=${QPKG_ID} -flto)
    target_link_libraries(qpkg PRIVATE yaml-cpp boost_filesystem quix-qxir quix-parser quix-lexer quix-core)
    target_link_options(qpkg PRIVATE -flto)
    add_dependencies(qpkg quix-parser quix-lexer quix-core quix-qxir)
else()
    target_compile_options(qpkg PRIVATE  -DQPKG_ID="undefined")
    target_link_libraries(qpkg PRIVATE yaml-cpp boost_filesystem quix-qxir quix-parser quix-lexer quix-core -fsanitize=address -fsanitize=undefined )
    add_dependencies(qpkg quix-parser quix-lexer quix-core quix-qxir)
endif()

target_include_directories(qpkg PRIVATE ${CMAKE_SOURCE_DIR}/libquix-core/include
                                        ${CMAKE_SOURCE_DIR}/libquix-lexer/include
                                        ${CMAKE_SOURCE_DIR}/libquix-parser/include
                                        ${CMAKE_SOURCE_DIR}/libquix-qxir/include
)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
