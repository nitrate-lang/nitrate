cmake_minimum_required(VERSION 3.15)
project(qpkg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

#================== CALCULATE BUILD ID ==================
execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -type f -name "*.*" -exec sha256sum {} \; COMMAND sha256sum
  OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QPKG_ID "\"QPKG_${SHA256SUMS}\"")
message(STATUS "qpkg VersionId: ${QPKG_ID}")

file(GLOB_RECURSE QPKG_CXX_SOURCES "src/*.cc")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  #================== CREATE APPLICATION RELEASE BUILD ================
  add_executable(qpkg ${QPKG_CXX_SOURCES})
  target_include_directories(qpkg PRIVATE src ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-prep/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include
    ${CMAKE_SOURCE_DIR}/libquix-ir/include
    ${CMAKE_SOURCE_DIR}/libquix-codegen/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0/include)
  target_compile_options(qpkg PRIVATE
    -Wpedantic
    -Wno-error=overlength-strings
    -DQPKG_ID=${QPKG_ID}
  )
  target_link_libraries(qpkg PUBLIC libboost_filesystem.a
    quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)
  target_link_options(qpkg PRIVATE -static-libgcc -static-libstdc++)
  add_dependencies(qpkg quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)

  #================== CREATE LIBRARY RELEASE BUILD ================
  add_library(qpkg-cmd ${QPKG_CXX_SOURCES})
  target_include_directories(qpkg-cmd PRIVATE src ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-prep/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include
    ${CMAKE_SOURCE_DIR}/libquix-ir/include
    ${CMAKE_SOURCE_DIR}/libquix-codegen/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0/include)
  target_compile_options(qpkg-cmd PRIVATE
    -Wpedantic
    -Wno-error=overlength-strings
    -DQPKG_ID=${QPKG_ID}
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -DQPKG_LIBRARY)
  target_link_libraries(qpkg-cmd PUBLIC libboost_filesystem.a
    quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)
  target_link_options(qpkg-cmd PRIVATE
    -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  add_dependencies(qpkg-cmd quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)
  set_target_properties(qpkg-cmd PROPERTIES OUTPUT_NAME qpkg)
else()
  #================== CREATE APPLICATION DEBUG BUILD ================
  add_executable(qpkg ${QPKG_CXX_SOURCES})
  target_include_directories(qpkg PRIVATE src ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-prep/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include
    ${CMAKE_SOURCE_DIR}/libquix-ir/include
    ${CMAKE_SOURCE_DIR}/libquix-codegen/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0/include)
  target_compile_options(qpkg PRIVATE
    -Wpedantic
    -Wno-error=overlength-strings
    "-DQPKG_ID=\"undefined\"")
  target_link_libraries(qpkg PUBLIC libboost_filesystem.a
    quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)
  target_link_options(qpkg PRIVATE -static-libgcc
    -static-libstdc++)
  add_dependencies(qpkg quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)

  #================== CREATE LIBRARY DEBUG BUILD ================
  add_library(qpkg-cmd ${QPKG_CXX_SOURCES})
  target_include_directories(qpkg-cmd PRIVATE src ${CMAKE_SOURCE_DIR}/libquix-core/include
    ${CMAKE_SOURCE_DIR}/libquix-lexer/include
    ${CMAKE_SOURCE_DIR}/libquix-prep/include
    ${CMAKE_SOURCE_DIR}/libquix-parser/include
    ${CMAKE_SOURCE_DIR}/libquix-ir/include
    ${CMAKE_SOURCE_DIR}/libquix-codegen/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp-0.8.0/include)
  target_compile_options(qpkg-cmd PRIVATE -Wpedantic
    -Wno-error=overlength-strings
    "-DQPKG_ID=\"undefined\""
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -DQPKG_LIBRARY)
  target_link_libraries(qpkg-cmd PUBLIC libboost_filesystem.a
    quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp
    yaml-cpp)
  target_link_options(qpkg-cmd PRIVATE -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
  add_dependencies(qpkg-cmd quix-codegen
    quix-qxir
    quix-parser
    quix-lexer
    quix-prep
    quix-core
    yaml-cpp)
  set_target_properties(qpkg-cmd PROPERTIES OUTPUT_NAME qpkg)
endif()

install(TARGETS qpkg-cmd DESTINATION lib)
install(TARGETS qpkg DESTINATION bin)
