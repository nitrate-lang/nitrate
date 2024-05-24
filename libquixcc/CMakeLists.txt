cmake_minimum_required(VERSION 3.15)
project(libquixcc CXX)

set(CMAKE_CXX_STANDARD 20)

# Find C++23 source files
file(GLOB_RECURSE LIBQUIX_CXX_SOURCES "src/*.cc")

# Get LLVM flags from llvm14-config --ldflags 
execute_process(COMMAND llvm14-config --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm14-config --link-static --libfiles
execute_process(COMMAND llvm14-config --link-static --libfiles
OUTPUT_VARIABLE LLVM_LIBS_RAW
OUTPUT_STRIP_TRAILING_WHITESPACE)

set(LLVM_LIBS_RAW "${LLVM_LIBS_RAW} /usr/lib/libzstd.a")

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS_RAW}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE "\n" " " LLVM_LIBS_BASH "${LLVM_LIBS_RAW}")

set(BUILD_FLAGS_COMMON -g -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-error=unused-parameter -Wno-error=unused-variable -Wno-predefined-identifier-outside-function)

SET(PROJECT_RELEASE_FLAGS -O3 -fvisibility=hidden -fvisibility-inlines-hidden -Wno-error=mismatched-new-delete -flto ${BUILD_FLAGS_COMMON} -DNDEBUG)

SET(PROJECT_DEBUG_FLAGS -Wno-error=unused-variable -Wno-error=mismatched-new-delete ${BUILD_FLAGS_COMMON})

SET(STATIC_PROJECT_RELEASE_FLAGS -O3 -Wno-error=mismatched-new-delete ${BUILD_FLAGS_COMMON})

add_library(libquixcc SHARED ${LIBQUIX_CXX_SOURCES})
target_include_directories(libquixcc PUBLIC "internal" include)
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(libquixcc PRIVATE ${PROJECT_RELEASE_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS -flto)
    target_link_options(libquixcc PRIVATE ${LLVM_LD_FLAGS} -O3 -flto -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
    add_custom_command(TARGET libquixcc POST_BUILD COMMAND ${CMAKE_STRIP} $<TARGET_FILE:libquixcc>)
else()
    target_compile_options(libquixcc PRIVATE ${PROJECT_DEBUG_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS)
    target_link_options(libquixcc PRIVATE ${LLVM_LD_FLAGS})
endif()

if (BUILD_TESTING)
    target_compile_options(libquixcc PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto --coverage)
    target_link_options(libquixcc PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto --coverage)
endif()

foreach(lib ${LLVM_LIBS})
    target_link_libraries(libquixcc PRIVATE ${lib})
endforeach()
target_link_libraries(libquixcc PRIVATE pthread boost_serialization execinfo crypto)
set_target_properties(libquixcc PROPERTIES OUTPUT_NAME quixcc)

add_library(quixcc-static STATIC ${LIBQUIX_CXX_SOURCES})
target_include_directories(quixcc-static PUBLIC "internal" include)
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(quixcc-static PRIVATE ${STATIC_PROJECT_RELEASE_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS)
    target_link_options(quixcc-static PRIVATE ${LLVM_LD_FLAGS} -O3 -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
else()
    target_compile_options(quixcc-static PRIVATE ${PROJECT_DEBUG_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS)
    target_link_options(quixcc-static PRIVATE ${LLVM_LD_FLAGS})
endif()

if (BUILD_TESTING)
    target_compile_options(quixcc-static PRIVATE -fprofile-arcs -ftest-coverage -O3 --coverage)
    target_link_options(quixcc-static PRIVATE -fprofile-arcs -ftest-coverage -O3 --coverage)
endif()

add_custom_command(TARGET quixcc-static POST_BUILD COMMAND sh -c "armerge --output libquixcc.a $<TARGET_FILE:quixcc-static> /usr/lib/libexecinfo.a /usr/lib/libcrypto.a ${LLVM_LIBS_RAW}")

set_target_properties(quixcc-static PROPERTIES OUTPUT_NAME quixcc)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()

file(GLOB_RECURSE TEST_PROGRAMS "test/programs/*.cc")

message(STATUS "Found test programs: ${TEST_PROGRAMS}")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

foreach(TEST_SOURCE ${TEST_PROGRAMS})
    message(STATUS "Adding test: ${TEST_SOURCE}")
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(qcc-${TEST_NAME} ${TEST_SOURCE})
    target_include_directories(qcc-${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test/programs)
    target_link_libraries(qcc-${TEST_NAME} quixcc-static z)
    add_dependencies(qcc-${TEST_NAME} quixcc-static)

    if (BUILD_TESTING)
        target_link_options(qcc-${TEST_NAME} PRIVATE --coverage)
    endif()
endforeach()