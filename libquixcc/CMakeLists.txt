cmake_minimum_required(VERSION 3.15)
project(libquixcc CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++23 source files
file(GLOB_RECURSE LIBQUIX_CXX_SOURCES "src/*.cc")

# Get LLVM flags from llvm-config-14 --ldflags 
execute_process(COMMAND llvm-config-14 --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm-config-14 --link-static --libfiles
execute_process(COMMAND llvm-config-14 --link-static --system-libs --libfiles
OUTPUT_VARIABLE LLVM_LIBS
OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")

SET(PROJECT_RELEASE_FLAGS -g -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-error=unused-parameter -Wno-error=unused-variable -fPIC -O3 -fvisibility=hidden -fvisibility-inlines-hidden -Wno-error=mismatched-new-delete -flto)

SET(PROJECT_DEBUG_FLAGS -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-error=unused-parameter -pedantic -Wno-error=unused-variable -fPIC -Wno-error=mismatched-new-delete)

SET(STATIC_PROJECT_RELEASE_FLAGS -g -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-error=unused-parameter-Wno-error=unused-variable -fPIC -O3  -Wno-error=mismatched-new-delete)

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
    target_compile_options(libquixcc PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto)
    target_link_options(libquixcc PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto)
endif()

foreach(lib ${LLVM_LIBS})
    target_link_libraries(libquixcc PRIVATE ${lib})
endforeach()
target_link_libraries(libquixcc PRIVATE pthread boost_serialization)
set_target_properties(libquixcc PROPERTIES OUTPUT_NAME quixcc)

add_library(libquixcc-static STATIC ${LIBQUIX_CXX_SOURCES})
target_include_directories(libquixcc-static PUBLIC "internal" include)
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(libquixcc-static PRIVATE ${STATIC_PROJECT_RELEASE_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS)
    target_link_options(libquixcc-static PRIVATE ${LLVM_LD_FLAGS} -O3 -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
else()
    target_compile_options(libquixcc-static PRIVATE ${PROJECT_DEBUG_FLAGS} -DLLVM_SUUPORT_ALL_TARGETS)
    target_link_options(libquixcc-static PRIVATE ${LLVM_LD_FLAGS})
endif()
set_target_properties(libquixcc-static PROPERTIES OUTPUT_NAME quixcc)

if (BUILD_TESTING)
    target_compile_options(libquixcc-static PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto)
    target_link_options(libquixcc-static PRIVATE -fprofile-arcs -ftest-coverage -O3 -flto)
endif()

if (BUILD_TESTING)
    add_subdirectory(test)
endif()