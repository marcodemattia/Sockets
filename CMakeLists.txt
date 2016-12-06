cmake_minimum_required(VERSION 3.0)
project(Sockets)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES src/StreamingServerWithSelect.cpp src/StreamingServerWithSelect.h src/StreamingServerWithPoll.cpp
        src/StreamingServerWithPoll.h src/DatagramServerWithPoll.cpp src/DatagramServerWithPoll.h
        src/MulticastServerWithPoll.cpp src/MulticastServerWithPoll.h)
add_executable(Server main.cpp ${SOURCE_FILES})
add_executable(TCPClient mainClient.cpp ${SOURCE_FILES})
add_executable(UDPClient mainUDPClient.cpp ${SOURCE_FILES})
add_executable(MulticastClient mainMulticastClient.cpp ${SOURCE_FILES})
