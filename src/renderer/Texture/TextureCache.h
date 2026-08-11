#pragma once

namespace synapse {

class TextureCache
{
public:
    TextureCache() = default;
    ~TextureCache() = default;

    TextureCache(const TextureCache&) = delete;
    TextureCache& operator=(const TextureCache&) = delete;
};

} // namespace synapse
