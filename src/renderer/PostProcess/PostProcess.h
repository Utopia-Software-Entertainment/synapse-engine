#pragma once

namespace synapse {

class PostProcess
{
public:
    PostProcess() = default;
    ~PostProcess() = default;

    PostProcess(const PostProcess&) = delete;
    PostProcess& operator=(const PostProcess&) = delete;
};

} // namespace synapse
