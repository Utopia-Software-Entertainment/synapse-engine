#pragma once

namespace synapse {

class Profiler
{
public:
    Profiler() = default;
    ~Profiler() = default;

    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
};

} // namespace synapse
