cmake_minimum_required(VERSION 3.15)
project(quixcc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find C++20 source files
file(GLOB_RECURSE QUIXCC_CXX_SOURCES "src/*.cc")

# # Get LLVM flags from llvm-config-14 --cxxflags
# execute_process(COMMAND llvm-config-14 --cxxflags
#                 OUTPUT_VARIABLE LLVM_CXX_FLAGS
#                 OUTPUT_STRIP_TRAILING_WHITESPACE)

# # Get LLVM flags from llvm-config-14 --ldflags 
# execute_process(COMMAND llvm-config-14 --ldflags
# OUTPUT_VARIABLE LLVM_LD_FLAGS
# OUTPUT_STRIP_TRAILING_WHITESPACE)

# # Get LLVM flags from llvm-config-14  --libs
# execute_process(COMMAND llvm-config-14 --libs --system-libs
# OUTPUT_VARIABLE LLVM_LIBS
# OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(quixcc ${QUIXCC_CXX_SOURCES})
target_include_directories(quixcc PRIVATE "src" ${CMAKE_CURRENT_SOURCE_DIR}/../libquixcc/include)

if (CMAKE_BUILD_TYPE STREQUAL "Release") 
    target_compile_options(quixcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror -O3 -flto)
    target_link_options(quixcc PRIVATE -flto)
    add_custom_command(TARGET quixcc POST_BUILD COMMAND strip $<TARGET_FILE:quixcc>)
else()
    target_compile_options(quixcc PRIVATE -g -Wall -Wextra -Wpedantic -Werror)
endif()
target_link_directories(quixcc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../libquixcc)
target_link_libraries(quixcc PRIVATE libquixcc ${LLVM_LIBS} ${LLVM_LD_FLAGS} ${LLVM_CXX_FLAGS} -lstdc++fs)
add_dependencies(quixcc libquixcc)

