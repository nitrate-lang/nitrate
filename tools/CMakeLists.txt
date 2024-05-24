cmake_minimum_required(VERSION 3.15)
project(quixcc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TOOLS "*.cc")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

foreach(TOOL ${TOOLS})
    get_filename_component(TOOL_NAME ${TOOL} NAME_WE)
    add_executable(quixcc-${TOOL_NAME} ${TOOL})
    target_include_directories(quixcc-${TOOL_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libquixcc/include)
    target_compile_options(quixcc-${TOOL_NAME} PRIVATE)
    target_link_libraries(quixcc-${TOOL_NAME} PRIVATE quixcc-static z)
    if (BUILD_TESTING)
        target_link_options(quixcc-${TOOL_NAME} PRIVATE --coverage)
    endif()
    add_dependencies(quixcc-${TOOL_NAME} quixcc-static)
endforeach()