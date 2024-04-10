cmake_minimum_required(VERSION 3.15)
project(libquixcc-tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TEST_PROGRAMS "programs/*.cc")

message(STATUS "Found test programs: ${TEST_PROGRAMS}")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

foreach(TEST_SOURCE ${TEST_PROGRAMS})
    message(STATUS "Adding test: ${TEST_SOURCE}")
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(qcc-${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(qcc-${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/programs)
    target_link_libraries(qcc-${TEST_NAME} libquixcc)
    add_dependencies(qcc-${TEST_NAME} libquixcc)
endforeach()

# Must rebuild the test if ${CMAKE_CURRENT_SOURCE_DIR}/data/manifest.json changes
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/src/generate_tests.py ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite-proto.cc ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/manifest.json ${CMAKE_CURRENT_SOURCE_DIR}/src/generate_tests.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMENT "Generating test suite"
)

add_executable(libquixcc-test-suite ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc)
target_link_libraries(libquixcc-test-suite GTest::gtest_main libquixcc crypto)
add_dependencies(libquixcc-test-suite libquixcc)
add_custom_target(libquixcc-test-suite-out ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/test-suite.cc)

add_test(NAME libquixcc-test-suite COMMAND libquixcc-test-suite)

include(GoogleTest)
gtest_discover_tests(libquixcc-test-suite)