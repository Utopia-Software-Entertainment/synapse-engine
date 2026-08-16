// Demo minimal 2.1/2.2 : HAL Platform + Window GLFW.
// Compile sans le renderer → build ciblé : apps/window_demo/window_demo
//
// Fermer la fenêtre (ou appuyer sur Échap) pour quitter.

#include <core/Engine.h>
#include <core/Logger.h>
#include <platform/Platform.h>
#include <platform/Window.h>
#include <platform/WindowEvents.h>

#include <GLFW/glfw3.h>

#include <chrono>

using namespace synapse;

int main()
{
    Logger::Init();
    Platform::Init();
    Engine engine;

    Window window("Synapse Engine — Window Demo", 1280, 720);

    u32 resizeCount = 0;
    window.SetResizeCallback([&](u32 width, u32 height) {
        ++resizeCount;
        SYNAPSE_CORE_INFO("Window resized → {}x{} ({} resize events)", width, height, resizeCount);
    });

    // Exercer le HAL : boucle à budget 90 Hz, fps affichés toutes les secondes.
    auto lastFpsTime = Platform::GetTimeMs();
    u32 frames = 0;
    u32 fps = 0;

    while (!window.ShouldClose())
    {
        const u64 frameStart = Platform::GetTimeMs();

        window.PollEvents();

        // Échap → fermer.
        if (glfwGetKey(window.GetHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window.GetHandle(), GLFW_TRUE);
        }

        ++frames;
        const u64 now = Platform::GetTimeMs();
        if (now - lastFpsTime >= 1000)
        {
            fps = frames;
            frames = 0;
            lastFpsTime = now;
            SYNAPSE_CORE_INFO("fps: {} | window: {}x{} | free RAM: {} MB",
                              fps, window.GetWidth(), window.GetHeight(),
                              Platform::AvailableMemoryMB());
        }

        // Budget 11 ms (~90 Hz) : dormir le reste de la frame.
        const u64 elapsed = Platform::GetTimeMs() - frameStart;
        constexpr u64 kBudgetMs = 11;
        if (elapsed < kBudgetMs)
        {
            Platform::Sleep(static_cast<u32>(kBudgetMs - elapsed));
        }
    }

    SYNAPSE_CORE_INFO("Window demo closed after {} frames (last fps: {})", frames, fps);
    Platform::Shutdown();
    return 0;
}