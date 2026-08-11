#include <core/Logger.h>

namespace synapse {

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

void Logger::Init()
{
    // Console sink (colored, multi-threaded)
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("%^[%T] %n: %v%$");

    // File sink (rotating, 5 MB max, 3 files)
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/engine.log", 5 * 1024 * 1024, 3
    );
    fileSink->set_pattern("[%Y-%m-%d %T] [%l] %n: %v");

    std::vector<spdlog::sink_ptr> sinks{consoleSink, fileSink};

    s_CoreLogger = std::make_shared<spdlog::logger>("SYNAPSE", sinks.begin(), sinks.end());
    s_CoreLogger->set_level(spdlog::level::trace);
    s_CoreLogger->flush_on(spdlog::level::trace);

    s_ClientLogger = std::make_shared<spdlog::logger>("AETHERIA", sinks.begin(), sinks.end());
    s_ClientLogger->set_level(spdlog::level::trace);
    s_ClientLogger->flush_on(spdlog::level::trace);

    spdlog::register_logger(s_CoreLogger);
    spdlog::register_logger(s_ClientLogger);
}

std::shared_ptr<spdlog::logger>& Logger::GetCoreLogger()
{
    return s_CoreLogger;
}

std::shared_ptr<spdlog::logger>& Logger::GetClientLogger()
{
    return s_ClientLogger;
}

} // namespace synapse
