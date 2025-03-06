cmake_minimum_required(VERSION 3.15)
project(libnitrate-emit CXX)

set(__TARGET_MAJOR_VERSION 0)
set(__TARGET_MINOR_VERSION 0)
set(__TARGET_PATCH_VERSION 0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MAJOR_VERSION=${__TARGET_MAJOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_MINOR_VERSION=${__TARGET_MINOR_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_PATCH_VERSION=${__TARGET_PATCH_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_HASH=\\\"${__TARGET_COMMIT_HASH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_BRANCH=\\\"${__TARGET_COMMIT_BRANCH}\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__TARGET_COMMIT_DATE=\\\"${__TARGET_COMMIT_DATE}\\\"")

#================== GET LLVM LIBRARIES =====================
execute_process(COMMAND llvm-config-18 --ldflags
  OUTPUT_VARIABLE LLVM_LD_FLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND llvm-config-18 --link-static --libfiles
  OUTPUT_VARIABLE LLVM_LIBS_RAW
  OUTPUT_STRIP_TRAILING_WHITESPACE)

string(REPLACE "\n" " " LLVM_LIBS "${LLVM_LIBS_RAW}")
string(REPLACE " " ";" LLVM_LIBS "${LLVM_LIBS}")
string(REPLACE "\n" " " LLVM_LIBS_BASH "${LLVM_LIBS_RAW}")

#################################################################

set(BUILD_FLAGS_COMMON
  "-Wno-deprecated-enum-enum-conversion  \
  -Wno-error=unused-but-set-variable     \
  -Wno-unused-parameter                  \
  -fno-exceptions                        \
                                         \
  -DTRANSCODE_TARGET_C11                 \
  -DTRANSCODE_TARGET_CXX11               \
  -DTRANSCODE_TARGET_TYPESCRIPT          \
  -DTRANSCODE_TARGET_RUST                \
  -DTRANSCODE_TARGET_PYTHON              \
  -DTRANSCODE_TARGET_CSHARP              \
  -DLLVM_SUUPORT_ALL_TARGETS")

find_library(libz z REQUIRED)
find_library(libzstd zstd REQUIRED)
find_library(libtinfo tinfo REQUIRED)

#================== SET BUILD FLAGS ==================
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BUILD_FLAGS_COMMON}")

file(GLOB_RECURSE CXX_SOURCES "src/*.cc")

add_library(nitrate-emit ${CXX_SOURCES})
target_include_directories(nitrate-emit PUBLIC src "include"
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-core/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../libnitrate-ir/include
  /usr/lib/llvm-18/include)
target_link_libraries(nitrate-emit PUBLIC nitrate-core nitrate-ir)
foreach(lib ${LLVM_LIBS})
  target_link_libraries(nitrate-emit PUBLIC ${lib})
  message(STATUS "Linking ${lib}")
endforeach()
target_link_libraries(nitrate-emit PUBLIC ${libz} ${libtinfo} ${libzstd})
add_dependencies(nitrate-emit nitrate-core nitrate-ir)
set_target_properties(nitrate-emit PROPERTIES OUTPUT_NAME nitrate-emit)

install(TARGETS nitrate-emit DESTINATION lib)
install(DIRECTORY include/ DESTINATION "include")
