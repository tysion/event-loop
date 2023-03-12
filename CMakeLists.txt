cmake_minimum_required(VERSION 3.19)
project(oxmel)

option(OXMEL_BUILD_EXAMPLE "Additionally build examples" OFF)
option(OXMEL_BUILD_TESTS "Additionally build unit tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

add_subdirectory(src)
link_libraries(atomic pthread)

if (OXMEL_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if (OXMEL_BUILD_TESTS)
    add_subdirectory(test)
endif()
