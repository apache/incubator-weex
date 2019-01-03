cmake_minimum_required(VERSION 3.4.1)

set(JSON11_LIBRARY_NAME json11)

add_library(${JSON11_LIBRARY_NAME} STATIC
  ./json11.cc
)

target_include_directories(${JSON11_LIBRARY_NAME} PUBLIC .)
