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

// Macros — null-safe (aucun crash si Logger::Init() n'a pas encore été appelé).
#define SYNAPSE_CORE_TRACE(...)    if (::synapse::Logger::GetCoreLogger())  { ::synapse::Logger::GetCoreLogger()->trace(__VA_ARGS__); }
#define SYNAPSE_CORE_INFO(...)     if (::synapse::Logger::GetCoreLogger())  { ::synapse::Logger::GetCoreLogger()->info(__VA_ARGS__); }
#define SYNAPSE_CORE_WARN(...)     if (::synapse::Logger::GetCoreLogger())  { ::synapse::Logger::GetCoreLogger()->warn(__VA_ARGS__); }
#define SYNAPSE_CORE_ERROR(...)    if (::synapse::Logger::GetCoreLogger())  { ::synapse::Logger::GetCoreLogger()->error(__VA_ARGS__); }
#define SYNAPSE_CORE_CRITICAL(...) if (::synapse::Logger::GetCoreLogger())  { ::synapse::Logger::GetCoreLogger()->critical(__VA_ARGS__); }

#define SYNAPSE_CLIENT_TRACE(...)  if (::synapse::Logger::GetClientLogger()) { ::synapse::Logger::GetClientLogger()->trace(__VA_ARGS__); }
#define SYNAPSE_CLIENT_INFO(...)   if (::synapse::Logger::GetClientLogger()) { ::synapse::Logger::GetClientLogger()->info(__VA_ARGS__); }
#define SYNAPSE_CLIENT_WARN(...)   if (::synapse::Logger::GetClientLogger()) { ::synapse::Logger::GetClientLogger()->warn(__VA_ARGS__); }
#define SYNAPSE_CLIENT_ERROR(...)  if (::synapse::Logger::GetClientLogger()) { ::synapse::Logger::GetClientLogger()->error(__VA_ARGS__); }
