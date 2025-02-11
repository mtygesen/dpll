cmake_minimum_required(VERSION 3.22)
project(dpll)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -fsanitize=address,undefined")
set(CMAKE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
set(CMAKE_BUILD_TYPE Release)

include_directories(include)
add_executable(dpll Main.cpp)