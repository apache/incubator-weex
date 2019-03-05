cmake_minimum_required(VERSION 3.4.1)
project(WeexCore)

add_compile_options(-fno-rtti)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fvisibility=hidden -g -Os -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -fvisibility=hidden -g -Os -ffunction-sections -fdata-sections")

add_subdirectory(Source)
