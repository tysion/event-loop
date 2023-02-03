cmake_minimum_required(VERSION 3.23)
project(oxmel)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES
    event_loop.cpp
    event_loop_context.cpp
    event_loop_context.h
    event_notificator_interface.h
    epoll_notificator.cpp
    epoll_notificator.h
    task.h ../include/oxm/event.h)

add_library(oxmel STATIC ${SOURCES})
target_include_directories(oxmel PRIVATE ${CMAKE_SOURCE_DIR}/include)
