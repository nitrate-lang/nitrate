project(nitrate-parser CXX)

set(TARGET_NAME "nitrate-parser")

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
  -Wold-style-cast
  -g3
  -fno-exceptions
  -fno-rtti
)

if(NITRATE_COMPILER_SANITIZE)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(${TARGET_NAME} ${LEXER_SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC nitrate-lexer spdlog)
target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
if(NITRATE_COMPILER_FUZZING)
  target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=fuzzer -fprofile-instr-generate -fcoverage-mapping)
endif()

install(TARGETS ${TARGET_NAME})
install(DIRECTORY "include/" DESTINATION "include")

add_executable(${TARGET_NAME}-test ${LEXER_TEST_SOURCES})
target_link_libraries(${TARGET_NAME}-test PRIVATE ${TARGET_NAME} gtest_main gtest spdlog)
add_test(NAME ${TARGET_NAME}-test COMMAND ${TARGET_NAME}-test)

add_executable(nitrate-parser-stdin tools/nitrate-parser-stdin.cc)
target_link_libraries(nitrate-parser-stdin PRIVATE ${TARGET_NAME})
install(TARGETS nitrate-parser-stdin)

add_executable(nitrate-parser-noout tools/nitrate-parser-noout.cc)
target_link_libraries(nitrate-parser-noout PRIVATE ${TARGET_NAME})
install(TARGETS nitrate-parser-noout)

if(NITRATE_COMPILER_FUZZING)
  add_executable(nitrate-parser-fuzz tools/nitrate-parser-fuzz.cc)
  target_link_libraries(nitrate-parser-fuzz PRIVATE ${TARGET_NAME} -fsanitize=fuzzer)
  install(TARGETS nitrate-parser-fuzz)
endif()
