add_executable(window_demo
    main.cpp
)

target_link_libraries(window_demo PRIVATE
    synapse_core
    synapse_platform
)

target_include_directories(window_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)