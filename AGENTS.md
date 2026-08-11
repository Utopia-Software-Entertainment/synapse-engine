# AGENTS.md — Synapse Engine

Moteur VRMMO neuro-visuel en C++23 (Utopia Software Entertainment). Réutilisable, ne contient AUCUN contenu de jeu. Propriété intellectuelle confidentielle.

## Commandes

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # configurer (FetchContent : EnTT, GLM, spdlog, GLFW, json, taskflow, VMA, enet, Jolt, Recast, fastgltf, imgui, whisper.cpp + find_package Vulkan/OpenXR REQUIRED)
cmake --build build -j                             # compiler
cmake --build build --target synapse_core -j       # cibler un module
ctest --test-dir build                             # tests (aucun pour l'instant)
```

Prérequis système : compilateur C++23, Ninja, Vulkan SDK, OpenXR SDK (voir SETUP.md).

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
| 08 Voice | `src/voice/` | Whisper.cpp (STT), MicCapture, Intent (parser, CommandMatcher) |
| 09 Network | `src/network/` | Transport (UDP, ReliableUDP), Serialization (bitstream, delta), Replication, Prediction, Server (BossStateMachine) |
| 10 Editor | `src/editor/` | ImGui (EditorLayer, backend Vulkan), Inspector, SceneSerializer, LogConsole |

Promesses clés : interface neurale zéro manette, rendu fovéal Vulkan (économies 50–70 %), ECS EnTT déterministe (milliers d'entités), modules remplaçables. Mode allongé (tête posée, regard = visée) : cas d'usage central, voir P0 4.6.

## État d'avancement — IMPORTANT

- **Squelette complet** : les 174 fichiers de tous les modules existent, mais la majorité sont des enveloppes vides (~15–35 lignes, classes sans implémentation). Total ~2 000 lignes.
- Seul `core/` a un début d'implémentation réelle (Engine ~74 lignes, Timer, Logger, EventBus, SystemScheduler, allocators, UUID).
- **Aucun shader `.glsl`**, aucun binaire produit, aucun test réel (tests/ = CMakeLists avec garde par module).
- Exemples de stubs : `src/vr/RecumbentMode.h` (classe vide), tous les .h/.cpp OpenXR et renderer.

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