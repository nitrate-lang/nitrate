cmake_minimum_required(VERSION 3.15)
project(qcc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++20 source files
file(GLOB_RECURSE QUIXCC_CXX_SOURCES "src/*.cc")

add_executable(qcc ${QUIXCC_CXX_SOURCES})
target_include_directories(qcc PRIVATE "src" ${CMAKE_CURRENT_SOURCE_DIR}/../libquixcc/include)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND LIBQUIXCC_STATIC) 
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXE_LINKER_FLAGS "-static")

    target_compile_options(qcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror -O3 -flto)
    target_link_options(qcc PRIVATE -flto)
    add_custom_command(TARGET qcc POST_BUILD COMMAND strip $<TARGET_FILE:qcc>)
    target_link_libraries(qcc PRIVATE quixcc-static z -lstdc++fs)
    add_dependencies(qcc quixcc-static)
else()
    target_compile_options(qcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror)
    target_link_libraries(qcc PRIVATE quixcc -fsanitize=address -fsanitize=undefined )
    add_dependencies(qcc quixcc)
endif()
target_link_directories(qcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../libquixcc)

if (BUILD_TESTING)
    target_link_options(qcc PRIVATE --coverage)
    add_subdirectory(test)
endif()