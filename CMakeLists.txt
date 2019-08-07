cmake_minimum_required(VERSION 3.5)

project(raft)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

add_executable(ryw-test test.cpp)
add_executable(right-client right.cpp)

add_subdirectory(src)
add_subdirectory(test)

