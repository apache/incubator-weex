
add_executable(HelloTest HelloTest.cpp)
target_link_libraries(HelloTest gtest_main)



add_test(WeexTests HelloTest)