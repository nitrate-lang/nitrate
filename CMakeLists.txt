cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite)

if (BUILD_TESTING)
    enable_testing()
endif()

add_subdirectory(deps/libexecinfo)

add_subdirectory(libquixcc)
add_subdirectory(qcc)
add_subdirectory(qld)
add_subdirectory(qpkg)
add_subdirectory(tools)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()