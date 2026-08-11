#pragma once

namespace synapse {

class StereoRenderer
{
public:
    StereoRenderer() = default;
    ~StereoRenderer() = default;

    StereoRenderer(const StereoRenderer&) = delete;
    StereoRenderer& operator=(const StereoRenderer&) = delete;
};

} // namespace synapse
