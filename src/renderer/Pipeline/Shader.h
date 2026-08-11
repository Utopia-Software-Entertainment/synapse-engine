#pragma once

namespace synapse {

class Shader
{
public:
    Shader() = default;
    ~Shader() = default;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
};

} // namespace synapse
