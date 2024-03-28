cmake_minimum_required(VERSION 3.15)
project(libquixcc-tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TEST_PROGRAMS "programs/*.cc")

message(STATUS "Found test programs: ${TEST_PROGRAMS}")

foreach(TEST_SOURCE ${TEST_PROGRAMS})
    message(STATUS "Adding test: ${TEST_SOURCE}")
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(qcc-${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(qcc-${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/programs)
    target_link_libraries(qcc-${TEST_NAME} libquixcc libstdc++.a)
    add_test(NAME qcc-${TEST_NAME} COMMAND qcc-${TEST_NAME})
    add_dependencies(qcc-${TEST_NAME} libquixcc)
endforeach()
