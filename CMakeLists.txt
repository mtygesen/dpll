cmake_minimum_required(VERSION 3.22.1)
project(dpll)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
add_executable(dpll Main.cpp)