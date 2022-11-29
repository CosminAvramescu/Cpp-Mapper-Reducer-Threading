cmake_minimum_required(VERSION 3.22)
project(tema1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(tema1 main.cpp)
