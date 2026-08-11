#pragma once

namespace synapse {

class EEGParser
{
public:
    EEGParser() = default;
    ~EEGParser() = default;

    EEGParser(const EEGParser&) = delete;
    EEGParser& operator=(const EEGParser&) = delete;
};

} // namespace synapse
