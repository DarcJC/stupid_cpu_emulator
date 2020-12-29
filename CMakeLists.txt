cmake_minimum_required(VERSION 3.15)
project(b22)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(b22 main.cpp main.hpp)
