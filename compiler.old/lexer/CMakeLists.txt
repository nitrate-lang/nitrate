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
  -Wold-style-cast
  -g3
  -fno-exceptions
  -fno-rtti
  -fvisibility=hidden
)

if(NITRATE_COMPILER_SANITIZE)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(${TARGET_NAME} ${LEXER_SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC boost_system boost_container)
target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
if(NITRATE_COMPILER_FUZZING)
  target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=fuzzer -fprofile-instr-generate -fcoverage-mapping)
endif()

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

add_executable(nitrate-lexer-stdin tools/nitrate-lexer-stdin.cc)
target_link_libraries(nitrate-lexer-stdin PRIVATE ${TARGET_NAME})
install(TARGETS nitrate-lexer-stdin)

add_executable(nitrate-lexer-noout tools/nitrate-lexer-noout.cc)
target_link_libraries(nitrate-lexer-noout PRIVATE ${TARGET_NAME})
install(TARGETS nitrate-lexer-noout)

if(NITRATE_COMPILER_FUZZING)
  add_executable(nitrate-lexer-fuzz tools/nitrate-lexer-fuzz.cc)
  target_link_libraries(nitrate-lexer-fuzz PRIVATE ${TARGET_NAME} -fsanitize=fuzzer)
  install(TARGETS nitrate-lexer-fuzz)
endif()
