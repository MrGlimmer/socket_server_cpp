cmake_minimum_required(VERSION 3.14)
project(socket_server_cpp)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(socket_server_cpp main.cpp Server.cpp Server.h)