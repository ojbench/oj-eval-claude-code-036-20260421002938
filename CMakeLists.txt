cmake_minimum_required(VERSION 3.10)
project(dynamic_bitset)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

# This assumes the OJ will provide a main.cpp or similar test file
# We create an executable named 'code' from all .cpp files
file(GLOB SOURCES "*.cpp")
add_executable(code ${SOURCES})
