macro(add_example name sources)
    set(binary_name ${CMAKE_PROJECT_NAME}_example_${name})
    add_executable(${binary_name} ${sources})
    target_link_libraries(${binary_name} ${CMAKE_PROJECT_NAME})
    target_include_directories(${binary_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    install(TARGETS ${binary_name}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endmacro()

add_example(simple simple.cpp)
add_example(tcp_printer tcp_printer.cpp)
