#pragma once

namespace synapse {

class XRInstance
{
public:
    XRInstance() = default;
    ~XRInstance() = default;

    XRInstance(const XRInstance&) = delete;
    XRInstance& operator=(const XRInstance&) = delete;
};

} // namespace synapse
