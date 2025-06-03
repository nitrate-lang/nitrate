project(nitrate-lexer CXX)

set(TARGET_NAME "nitrate-lexer")

file(GLOB_RECURSE LEXER_SOURCES "src/*.cc")
file(GLOB_RECURSE LEXER_TEST_SOURCES "test/*.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Werror
)

add_library(${TARGET_NAME} ${LEXER_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

install(TARGETS ${TARGET_NAME}
  EXPORT nitrate-compiler-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/nitrate-lexer"
  DESTINATION "include"
  FILES_MATCHING PATTERN "*.hh")

add_executable(${TARGET_NAME}-test ${LEXER_TEST_SOURCES})
target_link_libraries(${TARGET_NAME}-test PRIVATE ${TARGET_NAME} gtest_main gtest spdlog)
add_test(NAME ${TARGET_NAME}-test COMMAND ${TARGET_NAME}-test)
