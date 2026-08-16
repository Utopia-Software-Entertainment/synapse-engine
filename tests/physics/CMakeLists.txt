add_executable(synapse_physics_tests
    test_physics_world.cpp
    test_player_character.cpp
)

target_link_libraries(synapse_physics_tests PRIVATE
    synapse_physics
    doctest::doctest
)

add_test(NAME physics_tests COMMAND synapse_physics_tests)
