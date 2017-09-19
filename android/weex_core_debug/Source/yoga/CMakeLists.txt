cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-std=c99)

set(yogaSource
./YGEnums.c
./YGNodeList.c
./Yoga.c
)

add_library(yoga
STATIC
${yogaSource})

target_include_directories(yoga
PUBLIC
.)
