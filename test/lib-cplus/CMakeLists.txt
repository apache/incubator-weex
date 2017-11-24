cmake_minimum_required(VERSION 3.8)
project(WeexCore_FlexLayout)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../android/weex_core_debug/Source/Layout)
file(GLOB_RECURSE HEAD_FILES ${SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)
add_library(WXCoreLayout_lib ${HEAD_FILES} ${SOURCE_FILES})

include_directories(${SOURCE_DIR})
#add_executable(WeexCore_FlexLayout sample/main.cpp)
#target_link_libraries(WeexCore_FlexLayout WXCoreLayout_lib)

################################
# Testing
################################
# Options. Turn on with 'cmake -Dmyvarname=ON'.
# test option.
# coverage option.
option(COVERAGE "Use gcov" OFF)
IF (COVERAGE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF ()
if (APPLE)
  add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
  add_definitions(-D__GLIBCXX__)
endif (APPLE)

# This adds another subdirectory, which has 'project(gtest)'.
add_subdirectory(gtest/googletest)

# Include the gtest library. gtest_SOURCE_DIR is available due to
# 'project(gtest)' above.
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} .)

##############
# Unit Tests
##############
file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/case/*.cpp)

add_executable(runUnitTests ${TEST_FILES})

# Standard linking to gtest stuff.
target_link_libraries(runUnitTests gtest gtest_main)

# Extra linking for the project.
target_link_libraries(runUnitTests WXCoreLayout_lib)
