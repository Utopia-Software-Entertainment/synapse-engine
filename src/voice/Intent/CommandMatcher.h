#pragma once

namespace synapse {

class CommandMatcher
{
public:
    CommandMatcher() = default;
    ~CommandMatcher() = default;

    CommandMatcher(const CommandMatcher&) = delete;
    CommandMatcher& operator=(const CommandMatcher&) = delete;
};

} // namespace synapse
