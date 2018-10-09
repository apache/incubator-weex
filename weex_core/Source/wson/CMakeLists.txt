cmake_minimum_required(VERSION 3.4.1)

set(WSON_LIBRARY_NAME wson)

add_library(${WSON_LIBRARY_NAME} STATIC
  ./wson.c
  ./wson_util.cpp
  ./wson_parser.cpp
)

target_include_directories(${WSON_LIBRARY_NAME} PUBLIC .)
