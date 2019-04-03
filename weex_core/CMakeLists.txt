cmake_minimum_required(VERSION 3.4.1)
project(WeexCore)

add_compile_options(-fno-rtti)
add_compile_options(-funwind-tables)

if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fvisibility=hidden -g -Os -ffunction-sections -fdata-sections")
    set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -fvisibility=hidden -g -Os -ffunction-sections -fdata-sections")
    message(STATUS "************CMake Release Build************")
else()
    message(STATUS "************CMake ${CMAKE_BUILD_TYPE} Build************")
endif()

add_subdirectory(Source)
