#pragma once

namespace synapse {

class JawClenchDetector
{
public:
    JawClenchDetector() = default;
    ~JawClenchDetector() = default;

    JawClenchDetector(const JawClenchDetector&) = delete;
    JawClenchDetector& operator=(const JawClenchDetector&) = delete;
};

} // namespace synapse
