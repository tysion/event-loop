set(SOURCES
    driver.cpp
    event_test.cpp
    event_loop_test.cpp
    ${CMAKE_SOURCE_DIR}/src/core/simd_algorithm.cpp
    ${CMAKE_SOURCE_DIR}/src/multithreading/task_executor.cpp
    buddy_allocator_test.cpp
    lock_free_queue_test.cpp
    executor_test.cpp)

add_executable(${CMAKE_PROJECT_NAME}_test ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME}_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)

install(TARGETS ${CMAKE_PROJECT_NAME}_test
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)