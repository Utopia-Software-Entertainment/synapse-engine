add_library(synapse_network
    Transport/NetworkSocket.cpp
    Transport/ReliableUDP.cpp
    Serialization/Serializer.cpp
    Serialization/BitStream.cpp
    Serialization/DeltaEncoder.cpp
    Prediction/ClientPrediction.cpp
    Replication/ECSReplication.cpp
    Replication/InterestManager.cpp
    Server/ServerAISystem.cpp
    Server/BossStateMachine.cpp
)

target_link_libraries(synapse_network PUBLIC
    synapse_core
    enet
)
target_include_directories(synapse_network PUBLIC ${CMAKE_SOURCE_DIR}/src)
