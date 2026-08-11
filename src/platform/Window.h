#pragma once

namespace synapse {

class Window
{
public:
    Window() = default;
    ~Window() = default;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

} // namespace synapse
