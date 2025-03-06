cmake_minimum_required(VERSION 3.15)
project(libnitrate CXX)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate ${CXX_SOURCES})
target_include_directories(nitrate PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-lexer/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-seq/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-parser/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-ir/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-emit/include)
target_compile_options(nitrate PRIVATE ${PROJECT_DEBUG_FLAGS})
target_link_libraries(nitrate PUBLIC nitrate-core
  nitrate-lexer
  nitrate-seq
  nitrate-parser
  nitrate-ir
  nitrate-emit)
add_dependencies(nitrate nitrate-core nitrate-lexer nitrate-seq nitrate-parser nitrate-ir nitrate-emit)
set_target_properties(nitrate PROPERTIES OUTPUT_NAME nitrate)

install(TARGETS nitrate DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
