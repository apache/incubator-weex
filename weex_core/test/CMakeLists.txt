cmake_minimum_required(VERSION 3.4.1)
project(WeexCoreTest)
set(CMAKE_CXX_STANDARD 14)
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "************CMake Release Build************")
else()
    message(STATUS "************CMake ${CMAKE_BUILD_TYPE} Build************")
endif()


enable_testing()

add_subdirectory(third_party)
add_subdirectory(src)