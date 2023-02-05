set(SOURCES
    event_test.cpp
)

add_executable(${CMAKE_PROJECT_NAME}_test driver.cpp ${SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME}_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

