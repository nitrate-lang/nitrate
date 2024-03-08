cmake_minimum_required(VERSION 3.15)
project(libquixcc CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++23 source files
file(GLOB_RECURSE LIBQUIX_CXX_SOURCES "src/*.cc")

# Get LLVM flags from llvm-config-14 --cxxflags
execute_process(COMMAND llvm-config-14 --cxxflags
                OUTPUT_VARIABLE LLVM_CXX_FLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm-config-14 --ldflags 
execute_process(COMMAND llvm-config-14 --ldflags
OUTPUT_VARIABLE LLVM_LD_FLAGS
OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get LLVM flags from llvm-config-14  --libs
execute_process(COMMAND llvm-config-14 --libs --system-libs
OUTPUT_VARIABLE LLVM_LIBS
OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get random number
execute_process(COMMAND echo -n 0x | openssl rand -hex 4
OUTPUT_VARIABLE RANDOM_NUMBER
OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(PROJECT_RELEASE_FLAGS -Wall -Wextra -pedantic -Werror -Wno-error=unused-parameter ${LLVM_CXX_FLAGS} -frandom-seed=${RANDOM_NUMBER} -Wno-error=unused-variable -fPIC -O3 -s -fvisibility=hidden -fvisibility-inlines-hidden -Wno-error=mismatched-new-delete)
SET(PROJECT_DEBUG_FLAGS -g -Wall -Wextra -Werror -Wno-unused-parameter -Wno-error=unused-parameter -pedantic ${LLVM_CXX_FLAGS} -frandom-seed=${RANDOM_NUMBER} -Wno-error=unused-variable -fPIC  -Wno-error=mismatched-new-delete)

add_library(libquixcc SHARED ${LIBQUIX_CXX_SOURCES})
target_include_directories(libquixcc PUBLIC "internal" include)
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(libquixcc PRIVATE ${PROJECT_RELEASE_FLAGS} -flto)
    target_link_options(libquixcc PRIVATE ${LLVM_LD_FLAGS} -O3 -flto -Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/symexport.map)
    add_custom_command(TARGET libquixcc POST_BUILD COMMAND strip -s $<TARGET_FILE:libquixcc>)
else()
    target_compile_options(libquixcc PRIVATE ${PROJECT_DEBUG_FLAGS})
    target_link_options(libquixcc PRIVATE ${LLVM_LD_FLAGS})
endif()
target_link_libraries(libquixcc PRIVATE -lstdc++ -lm -lpthread -lstdc++fs crypto ${LLVM_LIBS})
set_target_properties(libquixcc PROPERTIES OUTPUT_NAME quixcc)

add_library(libquixcc_static STATIC ${LIBQUIX_CXX_SOURCES})
target_include_directories(libquixcc_static PUBLIC "internal" include)
if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(libquixcc_static PRIVATE ${PROJECT_RELEASE_FLAGS})
else()
    target_compile_options(libquixcc_static PRIVATE ${PROJECT_DEBUG_FLAGS})
    target_link_options(libquixcc_static PRIVATE ${LLVM_LD_FLAGS})
endif()
target_link_libraries(libquixcc_static PRIVATE -lstdc++ -lm -lpthread -lstdc++fs crypto ${LLVM_LIBS})
set_target_properties(libquixcc_static PROPERTIES OUTPUT_NAME quixcc)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()