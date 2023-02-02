cmake_minimum_required(VERSION 3.23)
project(event_loop)

set(CMAKE_CXX_STANDARD 20)

add_executable(event_loop main.cpp event_loop.cpp event_loop.h event_loop_context.cpp event_loop_context.h event_notificator_interface.h epoll_notificator.cpp epoll_notificator.h)
