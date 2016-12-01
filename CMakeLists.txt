cmake_minimum_required(VERSION 3.6)
project(Sockets)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/StreamingServerWithSelect.cpp src/StreamingServerWithSelect.h src/StreamingServerWithPoll.cpp
        src/StreamingServerWithPoll.h src/DatagramServerWithPoll.cpp src/DatagramServerWithPoll.h)
add_executable(StreamingServer main.cpp ${SOURCE_FILES})
add_executable(StreamingClient mainClient.cpp ${SOURCE_FILES})
add_executable(UDPClient mainUDPClient.cpp ${SOURCE_FILES})