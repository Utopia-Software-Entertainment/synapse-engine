#pragma once

namespace synapse {

class GazeFilter
{
public:
    GazeFilter() = default;
    ~GazeFilter() = default;

    GazeFilter(const GazeFilter&) = delete;
    GazeFilter& operator=(const GazeFilter&) = delete;
};

} // namespace synapse
