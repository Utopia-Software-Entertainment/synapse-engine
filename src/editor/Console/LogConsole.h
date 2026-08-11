#pragma once

namespace synapse {

class LogConsole
{
public:
    LogConsole() = default;
    ~LogConsole() = default;

    LogConsole(const LogConsole&) = delete;
    LogConsole& operator=(const LogConsole&) = delete;
};

} // namespace synapse
