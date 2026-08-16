# AGENTS.md — Synapse Engine

Moteur VRMMO neuro-visuel en C++23 (Utopia Software Entertainment). Réutilisable, ne contient AUCUN contenu de jeu. Propriété intellectuelle confidentielle.

## Commandes

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # configurer (FetchContent : EnTT, GLM, spdlog, GLFW, json, taskflow, VMA, enet, Jolt, Recast, fastgltf, imgui, whisper.cpp + find_package Vulkan/OpenXR REQUIRED)
cmake --build build -j                             # compiler
cmake --build build --target synapse_core -j       # cibler un module
cmake --build build --target synapse_core_tests -j # cibler les tests core
./build/tests/core/synapse_core_tests              # lancer les tests core (doctest)
ctest --test-dir build                             # tous les tests
```

Prérequis système : compilateur C++23, Ninja, Vulkan SDK, OpenXR SDK (voir SETUP.md).

## AI Toolchain — configurable

- `config/hardware.json` : profil PC détecté (RTX 4060 8 Go, 14 Go RAM, Ryzen 7 7735H). Régénérer avec `scripts/tools/detect_hardware.sh`.
- `config/ai_toolchain.json` : registre des outils IA (voir `docs/ai-toolchain.md`). Éditez les flags/paths puis relancez `scripts/tools/setup_ai_toolchain.sh`.
- Scripts : `scripts/tools/` (`install_prereqs.sh`, `fetch_models.sh`, `build_crisper_cli.sh`, `serve_qwen.sh`, `run_wan_animate.sh`, `run_minimax_h3.sh`, `setup_ai_toolchain.sh`).

## Architecture — 10 modules, dépendances unidirectionnelles

| Module | Dossier | Contenu |
|---|---|---|
| 01 Core & ECS | `src/core/` | Engine, game loop, SystemScheduler, Timer, EventBus, Logger, allocators (Arena/Pool/Stack), UUID, Types |
| 02 Platform | `src/platform/` | HAL (Platform), Window GLFW, InputManager, Profiler |
| 03 Renderer | `src/renderer/` | Vulkan (context/device/queues), Pipeline/Shader, Stereo, Foveated, Forward+, Culling, Shadow, PostProcess, Mesh, Texture |
| 04 VR | `src/vr/` | OpenXR (Instance/Session/HeadTracker/Projection), EyeTracker (gaze, Kalman), RecumbentMode, PostureCalibration |
| 05 Physics | `src/physics/` | Jolt (PhysicsWorld, RigidBody, Collision), IK (FABRIK), Navigation (NavMesh, pathfinding) |
| 06 Audio | `src/audio/` | AudioEngine, AudioSource, AudioSystem, HRTF, Occlusion |
| 07 Neural | `src/neural/` | LSL, EEG (parser, filtre, FFT, focus), EMG (blink, jaw clench), BioMapping |
| 08 Voice | `src/voice/` | Whisper.cpp (STT), MicCapture, Intent (parser, CommandMatcher), STT pipeline (STTPipeline + WhisperBackend + CrisperWhisperBackend optionnel) |
| 09 Network | `src/network/` | Transport (UDP, ReliableUDP), Serialization (bitstream, delta), Replication, Prediction, Server (BossStateMachine) |
| 10 Editor | `src/editor/` | ImGui (EditorLayer, backend Vulkan), Inspector, SceneSerializer, LogConsole |

Promesses clés : interface neurale zéro manette, rendu fovéal Vulkan (économies 50–70 %), ECS EnTT déterministe (milliers d'entités), modules remplaçables. Mode allongé (tête posée, regard = visée) : cas d'usage central, voir P0 4.6.

## État d'avancement — IMPORTANT

- **Squelette complet** : les 174 fichiers de tous les modules existent, mais la majorité sont des enveloppes vides (~15–35 lignes, classes sans implémentation). Total ~2 000 lignes.
- `core/` : implémentation réelle (Engine, Timer, Logger idempotent, EventBus, SystemScheduler, allocators, UUID) + **TieredScheduler** (3 niveaux, taskflow), **Math SIMD** (Vec/Mat4/Quat/Ray/Frustum, GLM), **ECS EnTT** (Entity/Registry/Components), **1.9 game loop multi-thread** (`ThreadPool` + `DoubleBuffer` + `MainLoop` 3 threads, budget 11 ms).
- `platform/` : **HAL fonctionnel** (`Platform.h/.cpp` : OS, cores, RAM, cwd, sleep, time) + **Window GLFW** (create/resize/PollEvents, API NO_API pour Vulkan, `WindowEvents.h`).
- `voice/` : pipeline STT fonctionnel (STTPipeline + WhisperBackend, timestamps mot-à-mot, mode verbatim) ; modèle ggml-small.bin téléchargé dans `assets/models/whisper/`. Backend **principal** CrisperWhisper (CLI buildé, poids non-commerciaux dans `assets/models/crisper/`) — actif via `config/ai_toolchain.json` (`"primary": true`), **jamais dans le build commercial**.
- Démo fenêtre : `apps/window_demo/` (core+platform seulement, ~92 fps sur budget 90 Hz) — `cmake --build build --target window_demo`.
- `physics/` : **monde Jolt fonctionnel** (5.1-5.2) — `PhysicsWorld` (JobSystem multi-thread, layers, gravité), `RigidBody` (Box/Sphere/Capsule, static/dynamic) + **`PlayerCharacter`** (capsule `CharacterVirtual`, déplacement WASD, **ne traverse pas les objets**), tests doctest.
- Démo complète `synapse_engine` : **renderer Vulkan fonctionnel** (3.1-3.3) — instance + validation layers, surface, GPU (AMD Radeon 680M via RADV), device swapchain, swapchain 5 images MAILBOX, MSAA 4x, shadow map 2048², skybox cubemap 128², texture mipmapée + aniso, pipeline graphique, mesh sphere.obj + instances, descripteurs. Monolithe dans `Renderer.cpp` (les classes `VulkanContext/VulkanDevice/Swapchain/Pipeline` restent des enveloppes) — zéro erreur de validation sur la démo. **La caméra est un personnage physique (Jolt) — on ne traverse pas les objets.**
- **Aucun shader `.glsl`** ne manque : compilation GLSL→SPIR-V→header embed via `glslc` + `scripts/embed_shader.cmake` (target `synapse_shaders`).
- Exemples de stubs : `src/vr/RecumbentMode.h` (classe vide), tous les .h/.cpp OpenXR.

## TODO

- `todo-p0.md` — bloquant (~93 j) : Core & ECS (1.1–1.9), Platform (2.1–2.2), Renderer (3.1–3.5, 3.12–3.13), VR (4.1–4.6, 4.8), Physics (5.1–5.2), Audio (6.1–6.3), Neural (7.1–7.3, 7.6–7.8), Voice (8.1–8.3), Network (9.1–9.2)
- `todo-p1.md`, `todo-p2.md`, `todo-p3.md` — suite
- Vertical slice cible : Core → ECS → renderer Vulkan → OpenXR → Jolt

## Conventions

- C++23, `std` strict, en-têtes `#pragma once`, namespace `synapse`.
- DOD (data-oriented design), layout cache-friendly, EnTT pour l'ECS.
- Rendu : toujours anticiper la stéréo VR (ceil gauche/droit, VK_KHR_multiview).
- Code Google-style (clang-format), `lint_on_save`/`format_on_save` activés dans `.opencode/config.json`.
- Docs de référence du jeu : repo voisin `Aetheria-Frontier` (GDD, CONTROLS.md — mode allongé, PVP.md, etc.).