#pragma once

namespace synapse {

class XRProjection
{
public:
    XRProjection() = default;
    ~XRProjection() = default;

    XRProjection(const XRProjection&) = delete;
    XRProjection& operator=(const XRProjection&) = delete;
};

} // namespace synapse
