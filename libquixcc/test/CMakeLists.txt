cmake_minimum_required(VERSION 3.15)
project(libquixcc-tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


file(GLOB_RECURSE TEMPLATES "${CMAKE_CURRENT_SOURCE_DIR}/data/inputs/*")

# Must rebuild the test if every time
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc
    COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/src/generate_tests.py ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite-proto.cc ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/manifest.json ${CMAKE_CURRENT_SOURCE_DIR}/src/generate_tests.py ${TEMPLATES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMENT "Generating test suite"
)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(libquixcc-test-suite ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc)
target_link_libraries(libquixcc-test-suite GTest::gtest_main quixcc-static z crypto)
target_link_options(libquixcc-test-suite PRIVATE --coverage)
add_dependencies(libquixcc-test-suite quixcc-static)
target_include_directories(libquixcc-test-suite PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_custom_target(libquixcc-test-suite-out ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc)

add_test(NAME libquixcc-test-suite COMMAND libquixcc-test-suite)

include(GoogleTest)
gtest_discover_tests(libquixcc-test-suite)