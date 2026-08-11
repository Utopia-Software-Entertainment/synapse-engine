#pragma once

namespace synapse {

class KalmanFilter
{
public:
    KalmanFilter() = default;
    ~KalmanFilter() = default;

    KalmanFilter(const KalmanFilter&) = delete;
    KalmanFilter& operator=(const KalmanFilter&) = delete;
};

} // namespace synapse
