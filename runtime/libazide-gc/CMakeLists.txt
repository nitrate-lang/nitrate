project(azide-gc CXX)

set(TARGET_NAME "azide-gc")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CARGO_BUILD_MODE "--debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(CARGO_BUILD_MODE "--release")
elseif(NOT CMAKE_BUILD_TYPE)
  set(CARGO_BUILD_MODE "--debug")
  message(STATUS "CMAKE_BUILD_TYPE is not set, defaulting to Debug.")
else()
  message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}. Supported types are Debug and Release.")
endif()

add_custom_target(${TARGET_NAME} ALL
  COMMAND cargo build ${CARGO_BUILD_MODE} --target-dir ${CMAKE_CURRENT_BINARY_DIR}/cargo
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  VERBATIM
)

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_BINARY_DIR}/cargo/release/libazide_gc.so
  ${CMAKE_CURRENT_BINARY_DIR}/libazide-gc.so
)

install(DIRECTORY "include/" DESTINATION "include")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libazide-gc.so
  DESTINATION lib
)
