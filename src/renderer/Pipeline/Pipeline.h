#pragma once

namespace synapse {

class Pipeline
{
public:
    Pipeline() = default;
    ~Pipeline() = default;

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
};

} // namespace synapse
