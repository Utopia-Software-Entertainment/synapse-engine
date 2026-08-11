#pragma once

namespace synapse {

class STTPipeline
{
public:
    STTPipeline() = default;
    ~STTPipeline() = default;

    STTPipeline(const STTPipeline&) = delete;
    STTPipeline& operator=(const STTPipeline&) = delete;
};

} // namespace synapse
