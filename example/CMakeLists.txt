macro(add_example name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} oxmel)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    install(TARGETS ${name}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endmacro()

add_example(simple simple.cpp)
add_example(tcp_printer tcp_printer.cpp)
