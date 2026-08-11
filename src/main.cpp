#include <core/Engine.h>
#include <platform/Window.h>
#include <renderer/Vulkan/Renderer.h>

#include <chrono>
#include <cmath>

#include <glm/gtc/constants.hpp>

auto main() -> int
{
    synapse::Engine engine;
    synapse::Window window("Synapse Engine", 1280, 720);
    synapse::Renderer renderer(window, 1280, 720);

    engine.Start();

    auto startTime = std::chrono::steady_clock::now();

    while (engine.Running() && !window.ShouldClose())
    {
        window.PollEvents();

        const float elapsed = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count();
        const float hue = std::fmod(elapsed * 0.1f, 1.0f);
        renderer.SetClearColor(glm::vec3(0.5f + 0.5f * std::sin(hue * glm::two_pi<float>()),
                                         0.3f + 0.3f * std::sin(hue * glm::two_pi<float>() + 2.094f),
                                         0.5f + 0.5f * std::sin(hue * glm::two_pi<float>() + 4.188f)));

        renderer.Draw();
        engine.TickFrame();
    }

    return 0;
}