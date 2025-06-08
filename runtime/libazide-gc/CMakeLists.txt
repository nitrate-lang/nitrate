project(azide-gc CXX)

set(TARGET_NAME "azide-gc")

file(GLOB_RECURSE AZIDE_GC_SOURCES "src/*.cc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Werror
  -Wold-style-cast
  -g3
  -fno-exceptions
  -fno-rtti
)

if(NITRATE_COMPILER_SANITIZE)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(${TARGET_NAME} ${AZIDE_GC_SOURCES})
target_link_libraries(${TARGET_NAME} PUBLIC)
target_include_directories(${TARGET_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
if(NITRATE_COMPILER_FUZZING)
  target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=fuzzer -fprofile-instr-generate -fcoverage-mapping)
endif()

install(TARGETS ${TARGET_NAME})
install(DIRECTORY "include/" DESTINATION "include")

if(NITRATE_COMPILER_FUZZING)
endif()
