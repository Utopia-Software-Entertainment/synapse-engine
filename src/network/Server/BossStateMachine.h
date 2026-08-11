#pragma once

namespace synapse {

class BossStateMachine
{
public:
    BossStateMachine() = default;
    ~BossStateMachine() = default;

    BossStateMachine(const BossStateMachine&) = delete;
    BossStateMachine& operator=(const BossStateMachine&) = delete;
};

} // namespace synapse
