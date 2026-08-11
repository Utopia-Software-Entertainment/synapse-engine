#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>

namespace synapse {

class Logger
{
public:
    static void Init();
    static std::shared_ptr<spdlog::logger>& GetCoreLogger();
    static std::shared_ptr<spdlog::logger>& GetClientLogger();

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

} // namespace synapse

// Macros
#define SYNAPSE_CORE_TRACE(...)    ::synapse::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define SYNAPSE_CORE_INFO(...)     ::synapse::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define SYNAPSE_CORE_WARN(...)     ::synapse::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define SYNAPSE_CORE_ERROR(...)    ::synapse::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define SYNAPSE_CORE_CRITICAL(...) ::synapse::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define SYNAPSE_CLIENT_TRACE(...)  ::synapse::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define SYNAPSE_CLIENT_INFO(...)   ::synapse::Logger::GetClientLogger()->info(__VA_ARGS__)
#define SYNAPSE_CLIENT_WARN(...)   ::synapse::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define SYNAPSE_CLIENT_ERROR(...)  ::synapse::Logger::GetClientLogger()->error(__VA_ARGS__)
