cmake_minimum_required(VERSION 3.19)
project(CheckSumTest)

set(CMAKE_CXX_STANDARD 20)

add_executable(CheckSumTest main.cpp)

# Turn on testing
enable_testing()
# Test the test binary itself
add_test(NAME CheckSumTest COMMAND "${CMAKE_BINARY_DIR}/CheckSumTest")