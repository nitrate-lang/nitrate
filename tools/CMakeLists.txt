cmake_minimum_required(VERSION 3.15)
project(quixcc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE TOOLS "*.cc")

foreach(TOOL ${TOOLS})
    get_filename_component(TOOL_NAME ${TOOL} NAME_WE)
    add_executable(quixcc-${TOOL_NAME} ${TOOL})
    target_include_directories(quixcc-${TOOL_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libquixcc/include)
    target_link_libraries(quixcc-${TOOL_NAME} PRIVATE libquixcc)
endforeach()