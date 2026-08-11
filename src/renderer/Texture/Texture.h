#pragma once

namespace synapse {

class Texture
{
public:
    Texture() = default;
    ~Texture() = default;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
};

} // namespace synapse
