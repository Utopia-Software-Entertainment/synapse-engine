# Synapse Engine

**Synapse Engine** est un moteur VRMMO modulaire écrit en C++23, architecturé autour de quatre piliers :

- **Interface neurale** — Eye-tracking + BCI (EEG/EMG) + reconnaissance vocale offline comme seuls périphériques d'entrée. Zéro manette.
- **Rendu fovéal Vulkan** — Le regard dicte où la résolution GPU est maximale. Économie de 50-70% sur la bande passante, rendant le VR photoréaliste accessible sur des configurations modestes.
- **ECS déterministe** — Architecture entité-composant-système (EnTT) multi-threadée, pensée pour des milliers d'entités simultanées et une synchronisation réseau autoritaire.
- **Modulaire et remplaçable** — Chaque module (rendu, audio, réseau, BCI, VR) est abstrait derrière une interface. Aucune dépendance verrouillée.

> **Ce dépôt ne contient PAS de contenu de jeu.** Synapse Engine est un moteur réutilisable, conçu pour servir de fondation à n'importe quel VRMMO.

---

## Architecture

```
synapse-engine/
├── core/           # Module 01 — ECS, logging, allocators, math, game loop
├── platform/       # Module 02 — Abstraction OS, windowing, input, profiling
├── renderer/       # Module 03 — Vulkan, shaders, foveated rendering, post-FX
├── vr/             # Module 04 — OpenXR, head/eye tracking, gaze raycast, recumbent mode
├── physics/        # Module 05 — Jolt Physics, rigid bodies, CCD, IK, navmesh
├── audio/          # Module 06 — Audio 3D spatialisé, HRTF, doppler, occlusion
├── neural/         # Module 07 — BCI (LSL/OpenBCI), EMG, EEG, focus analysis
├── voice/          # Module 08 — Whisper.cpp STT, intent parser
├── network/        # Module 09 — Transport UDP, prédiction client, server meshing
└── editor/         # Module 10 — ImGui debug layer, inspecteur ECS, scene serializer
```

Chaque module est indépendant et ne dépend que des modules précédents.

---

## Phases de développement

| Phase | Mois | Modules |
|-------|------|---------|
| **1** | 1—2 | Core & ECS, Platform |
| **2** | 3—6 | Renderer Vulkan, VR/OpenXR, Physics |
| **3** | 7—9 | Audio 3D, Neural Input (BCI), Voice AI |
| **4** | 10—14 | Network MMO (serveur autoritaire, meshing) |
| **5** | 14—16 | Editor & debug tools (ImGui) |

Charge : **3 développeurs** — durée estimée **~16 mois** pour le moteur complet.

---

## Dépendances externes

| Bibliothèque | Rôle | Module |
|-------------|------|--------|
| [EnTT](https://github.com/skypjack/entt) | Registre ECS | 01 |
| [spdlog](https://github.com/gabime/spdlog) | Journalisation | 01 |
| [GLM](https://github.com/g-truc/glm) | Mathématiques SIMD | 01 |
| [GLFW](https://github.com/glfw/glfw) | Fenêtrage cross-platform | 02 |
| [Vulkan SDK](https://vulkan.lunarg.com/) | Rendu graphique | 03 |
| [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | Mémoire GPU | 03 |
| [fastgltf](https://github.com/spnda/fastgltf) | Chargement glTF 2.0 | 03 |
| [OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK) | VR standard | 04 |
| [Jolt Physics](https://github.com/jrouwe/JoltPhysics) | Physique multi-threadée | 05 |
| [Recast/Detour](https://github.com/recastnavigation/recastnavigation) | Pathfinding | 05 |
| [OpenAL Soft](https://github.com/kcat/openal-soft) | Audio 3D spatialisé | 06 |
| [BrainFlow](https://github.com/brainflow-dev/brainflow) | Acquisition BCI | 07 |
| [liblsl](https://github.com/sccn/liblsl) | Streaming neuroscientifique | 07 |
| [whisper.cpp](https://github.com/ggerganov/whisper.cpp) | Reconnaissance vocale offline | 08 |
| [GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets) | Transport réseau | 09 |
| [Dear ImGui](https://github.com/ocornut/imgui) | Editor UI debug | 10 |

---

## Build

*À définir — CMake avec preset multi-config sera la cible.*

Prérequis : compilateur C++23, Vulkan SDK, OpenXR SDK.

```bash
git clone https://github.com/utopia-software-entertainment/synapse-engine
cd synapse-engine
# cmake --preset release && cmake --build build/release
```

---

## Roadmap

Les TODO listes par priorité sont dans les fichiers suivants :

- `todo-p0.md` — Tâches bloquantes (rien ne peut avancer sans)
- `todo-p1.md` — Tâches importantes (sprint courant)
- `todo-p2.md` — Tâches secondaires (sprint suivant)
- `todo-p3.md` — Polissage (à définir)

---

## License

Confidentiel — Utopia Software Entertainment
