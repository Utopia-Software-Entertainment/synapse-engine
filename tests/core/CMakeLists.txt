add_executable(synapse_core_tests
    main.cpp
    test_timer.cpp
    test_eventbus.cpp
    test_scheduler.cpp
    test_tiered_scheduler.cpp
    test_uuid.cpp
    test_allocators.cpp
    test_math.cpp
    test_handle.cpp
    test_ecs.cpp
    test_threadpool.cpp
    test_doublebuffer.cpp
    test_mainloop.cpp
)

target_link_libraries(synapse_core_tests PRIVATE
    synapse_core
    doctest::doctest
)

add_test(NAME core_tests COMMAND synapse_core_tests)
