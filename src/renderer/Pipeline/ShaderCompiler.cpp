#include <renderer/Pipeline/ShaderCompiler.h>

#include <core/Logger.h>

#include <array>
#include <cstdio>
#include <string>
#include <system_error>

namespace synapse {

bool ShaderCompiler::CompileToSpirv(std::string_view sourcePath, std::string_view stage,
                                    std::vector<uint8_t>& outSpirv)
{
    std::string command = "glslc -fshader-stage=";
    command.append(stage);
    command.append(" -o - ");
    command.append(sourcePath);
    command.append(" 2>/dev/null");

    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr)
    {
        SYNAPSE_CORE_ERROR("Shader hot-reload: impossible de lancer glslc pour {}", sourcePath);
        return false;
    }

    outSpirv.clear();
    std::array<char, 8192> buffer{};
    std::size_t read = 0;
    while ((read = fread(buffer.data(), 1, buffer.size(), pipe)) > 0)
    {
        outSpirv.insert(outSpirv.end(), buffer.data(), buffer.data() + read);
    }

    const int status = pclose(pipe);
    if (status != 0 || outSpirv.empty())
    {
        SYNAPSE_CORE_ERROR("Shader hot-reload: échec glslc pour {} (status {})", sourcePath, status);
        return false;
    }
    return true;
}

bool ShaderCompiler::GetModifiedTime(std::string_view path, std::filesystem::file_time_type& outTime)
{
    std::error_code ec;
    outTime = std::filesystem::last_write_time(std::filesystem::path(path), ec);
    return !ec;
}

} // namespace synapse
