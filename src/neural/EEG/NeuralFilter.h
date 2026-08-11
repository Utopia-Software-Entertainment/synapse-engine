#pragma once

namespace synapse {

class NeuralFilter
{
public:
    NeuralFilter() = default;
    ~NeuralFilter() = default;

    NeuralFilter(const NeuralFilter&) = delete;
    NeuralFilter& operator=(const NeuralFilter&) = delete;
};

} // namespace synapse
