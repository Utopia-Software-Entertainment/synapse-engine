#pragma once

namespace synapse {

class XRSession
{
public:
    XRSession() = default;
    ~XRSession() = default;

    XRSession(const XRSession&) = delete;
    XRSession& operator=(const XRSession&) = delete;
};

} // namespace synapse
