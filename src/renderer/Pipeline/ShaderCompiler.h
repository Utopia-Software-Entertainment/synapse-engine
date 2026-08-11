#pragma once

namespace synapse {

class ShaderCompiler
{
public:
    ShaderCompiler() = default;
    ~ShaderCompiler() = default;

    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
};

} // namespace synapse
