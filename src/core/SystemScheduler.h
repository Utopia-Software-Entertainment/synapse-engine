#pragma once

#include <core/Types.h>
#include <vector>
#include <functional>

namespace synapse {

using SystemFn = std::function<void(f32)>;

enum class SystemPhase : u8
{
    PrePhysics,
    Physics,
    PostPhysics,
    PreRender,
    Render,
    PostRender
};

struct SystemDesc
{
    SystemPhase phase;
    SystemFn    fn;
    const char* name;
};

class SystemScheduler
{
public:
    void Init();
    void RegisterSystem(SystemDesc&& desc);
    void ExecuteSystems(f32 deltaTime);

private:
    std::vector<std::vector<SystemDesc>> m_Phases;
};

} // namespace synapse
