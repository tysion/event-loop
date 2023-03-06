set(SOURCES
    event_loop.cpp
    event_loop_context.h
    io/base_notificator.h
    io/notificator.h
    io/linux/epoll_notificator.h
    io/linux/epoll_notificator.cpp
    memory/buddy_allocator.h
    memory/aligned_allocator.h
    memory/base_allocator.h task_allocator.h
    core/simd_algorithm.cpp
    core/simd_algorithm.h
        core/lock_free_queue.h)

set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/oxm/event.h
    ${CMAKE_SOURCE_DIR}/include/oxm/event_loop.h
    ${CMAKE_SOURCE_DIR}/include/oxm/task.h
)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(oxmel PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)

install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES ${PUBLIC_HEADERS} DESTINATION include/oxm)