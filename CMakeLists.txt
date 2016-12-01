cmake_minimum_required(VERSION 3.6)
project(Sockets)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp src/ServerWithSelect.cpp src/ServerWithSelect.h src/ServerWithPoll.cpp src/ServerWithPoll.h)
add_executable(Sockets ${SOURCE_FILES})