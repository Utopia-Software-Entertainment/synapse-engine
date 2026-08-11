#pragma once

namespace synapse {

class Serializer
{
public:
    Serializer() = default;
    ~Serializer() = default;

    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;
};

} // namespace synapse
