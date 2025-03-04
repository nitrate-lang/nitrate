cmake_minimum_required(VERSION 3.15)
project(no3-app CXX)

add_executable(no3 src/main.cc)
target_link_libraries(no3 PRIVATE no3-tool readline)
target_include_directories(no3 PRIVATE ${CMAKE_SOURCE_DIR}/no3/lib/include)
add_dependencies(no3 no3-tool)

install(TARGETS no3 DESTINATION bin)
