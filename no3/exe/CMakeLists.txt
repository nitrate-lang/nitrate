cmake_minimum_required(VERSION 3.15)
project(nitrate-tool-app CXX)

add_executable(no3 src/main.cc)

target_link_libraries(no3 PRIVATE nitrate-tool readline)
target_include_directories(no3 PRIVATE ${CMAKE_SOURCE_DIR}/no3/lib/include)

install(TARGETS no3 DESTINATION bin)
