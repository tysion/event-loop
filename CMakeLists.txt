cmake_minimum_required(VERSION 3.23)
project(event_loop)

set(CMAKE_CXX_STANDARD 20)

add_executable(event_loop main.cpp epoll.cpp epoll.h)
