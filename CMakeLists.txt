cmake_minimum_required(VERSION 3.15)
project(quix-compiler-suite)

option(ONLY_BUILD_LIBQUIXCC "Only build libquixcc" OFF)

if (BUILD_TESTING)
enable_testing()
endif()


# Calculate build ID
set(ENV_CONCAT_DATA "")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    string(CONCAT ENV_CONCAT_DATA "${ENV_CONCAT_DATA}${_variableName}=${${_variableName}}:")
endforeach()
execute_process(COMMAND find ${CMAKE_SOURCE_DIR} -type f -name "*.cc" -exec sha256sum {} \; COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND echo ${ENV_CONCAT_DATA} ${SHA256SUMS} COMMAND sha256sum
    OUTPUT_VARIABLE SHA256SUMS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE " .*" "" SHA256SUMS ${SHA256SUMS})
string(TOUPPER ${SHA256SUMS} SHA256SUMS)
string(SUBSTRING ${SHA256SUMS} 0 32 SHA256SUMS)
set(QUIX_BUILD_ID "\"QPKG_${SHA256SUMS}\"")
message(STATUS "Quix BuildID: ${QUIX_BUILD_ID}")

add_subdirectory(libquixcc)

if (NOT ONLY_BUILD_LIBQUIXCC)
  add_subdirectory(qcc)
  add_subdirectory(qld)
  add_subdirectory(qpkg)

  if (LIBQUIXCC_STATIC)
    add_subdirectory(tools)
  endif()
endif()