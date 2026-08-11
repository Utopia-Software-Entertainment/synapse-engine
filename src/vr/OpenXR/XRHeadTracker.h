#pragma once

namespace synapse {

class XRHeadTracker
{
public:
    XRHeadTracker() = default;
    ~XRHeadTracker() = default;

    XRHeadTracker(const XRHeadTracker&) = delete;
    XRHeadTracker& operator=(const XRHeadTracker&) = delete;
};

} // namespace synapse
