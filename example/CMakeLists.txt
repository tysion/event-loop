add_executable(example main.cpp)
target_link_libraries(example oxmel)
target_include_directories(example PRIVATE ${CMAKE_SOURCE_DIR}/include)


