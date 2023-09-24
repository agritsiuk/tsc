cmake_minimum_required(VERSION 3.10)
project(tsc)
set(CMAKE_CXX_STANDARD 17)

add_library(tsc INTERFACE)
target_include_directories(tsc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)
