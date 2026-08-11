#pragma once

namespace synapse {

class Platform
{
public:
    Platform() = default;
    ~Platform() = default;

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
};

} // namespace synapse
