#pragma once

namespace synapse {

class InputManager
{
public:
    InputManager() = default;
    ~InputManager() = default;

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
};

} // namespace synapse
