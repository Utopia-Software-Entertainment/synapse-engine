#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

namespace synapse {

class ShaderCompiler
{
public:
    static bool CompileToSpirv(std::string_view sourcePath, std::string_view stage,
                               std::vector<uint8_t>& outSpirv);
    static bool GetModifiedTime(std::string_view path, std::filesystem::file_time_type& outTime);
};

} // namespace synapse
