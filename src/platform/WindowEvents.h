#pragma once

#include <core/Types.h>

namespace synapse {

// ─────────────────────────────────────────────────────────────────────────
// WindowEvents — évènements fenêtre produits par Window::PollEvents()
// (todo 2.2). Consommés par InputManager et le rendu (resize → swapchain).
// ─────────────────────────────────────────────────────────────────────────

struct WindowResizeEvent
{
    u32 width;
    u32 height;
};

struct WindowCloseEvent
{
    bool requested = false;
};

struct WindowFocusEvent
{
    bool focused = true;
};

} // namespace synapse