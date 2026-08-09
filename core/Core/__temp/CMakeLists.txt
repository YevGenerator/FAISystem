cmake_minimum_required(VERSION 4.0)
project(Temp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Temp main.cpp
)
target_include_directories(Temp PRIVATE ../../Core)