set(SOURCES
    event_loop.cpp
    event_loop_context.cpp
    event_loop_context.h
    event_notificator_interface.h
    epoll_notificator.cpp
    epoll_notificator.h
    task.h
    registry.cpp
    registry.h)

set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/oxm/event.h
    ${CMAKE_SOURCE_DIR}/include/oxm/event_loop.h
)

add_library(${CMAKE_PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(oxmel PRIVATE ${CMAKE_SOURCE_DIR}/include)

install(TARGETS ${CMAKE_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES ${PUBLIC_HEADERS} DESTINATION include/oxm)