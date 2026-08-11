#pragma once

namespace synapse {

class ClientPrediction
{
public:
    ClientPrediction() = default;
    ~ClientPrediction() = default;

    ClientPrediction(const ClientPrediction&) = delete;
    ClientPrediction& operator=(const ClientPrediction&) = delete;
};

} // namespace synapse
