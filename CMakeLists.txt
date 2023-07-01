cmake_minimum_required(VERSION 3.10.2)
project(RISC_V)

include_directories()
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -Ofast")

add_executable(code ${src_dir} main.cpp)