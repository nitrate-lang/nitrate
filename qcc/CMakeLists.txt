cmake_minimum_required(VERSION 3.15)
project(qcc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE QUIXCC_CXX_SOURCES "src/*.cc")

add_executable(qcc ${QUIXCC_CXX_SOURCES})
target_include_directories(qcc PRIVATE "src" ${CMAKE_CURRENT_SOURCE_DIR}/../libquixcc/include)
target_link_directories(qcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../libquixcc)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(qcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror -O3 -flto)
    target_link_libraries(qcc PRIVATE quixcc libstdc++.a)
    add_custom_command(TARGET qcc POST_BUILD COMMAND strip $<TARGET_FILE:qcc>)
    add_dependencies(qcc quixcc)
else()
    target_compile_options(qcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror)
    target_link_libraries(qcc PRIVATE quixcc libstdc++.a -fsanitize=address -fsanitize=undefined)
    add_dependencies(qcc quixcc)
endif()

if (BUILD_TESTING)
    target_link_options(qcc PRIVATE --coverage)
    add_subdirectory(test)
endif()