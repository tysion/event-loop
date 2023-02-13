set(SOURCES
    driver.cpp
    event_test.cpp
    event_loop_test.cpp
    ${CMAKE_SOURCE_DIR}/src/event_loop_context.cpp
    ${CMAKE_SOURCE_DIR}/src/registry.cpp
    registry_test.cpp)

add_executable(${CMAKE_PROJECT_NAME}_test ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME}_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)

