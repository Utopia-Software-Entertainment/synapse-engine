# 🟡 SYNAPSE ENGINE — TODO P2 SECONDAIRE

> **Tâches du sprint suivant.**
> Durée totale estimée : **~46 jours**

---

## MODULE 02 — Platform & Windowing

- [ ] **2.3** Input Manager (éditeur seulement, clavier/souris GLFW) — `InputManager.h/.cpp`, `KeyCodes.h`, `MouseState.h` — **2j**
- [ ] **2.4** Profiler CPU Interne (PROFILE_SCOPE, timeline ImGui, Tracy/Optick) — `Profiler.h/.cpp`, `ProfilerScope.h`, `ProfilerUI.h` — **2j**

## MODULE 03 — Renderer Vulkan

- [ ] **3.11** Shadow Mapping (Cascaded Shadow Maps, lightmaps baked, auto GPU detect) — `ShadowPass.h/.cpp`, `CascadedShadowMap.h`, `LightmapLoader.h` — **4j**
- [ ] **3.14** Post-Processing (Bloom, Tone Mapping ACES, Color Grading, Chromatic Aberration, Vignette) — `PostProcess.h/.cpp`, `BloomPass.h`, `ToneMappingPass.h`, `ColorGradePass.h` — **4j**

## MODULE 04 — VR & Eye-Tracking

- [ ] **4.7** Reprojection Asynchrone (ASW) — frames synthétiques 45→90 FPS — `AsyncReprojection.h/.cpp`, `MotionVectorPass.h` — **5j**

## MODULE 05 — Physics

- [ ] **5.5** Full-Body IK (FABRIK, reconstruction corps mains+tête→squelette) — `BodyIKSolver.h/.cpp`, `FABRIKSolver.h`, `SkeletonComponent.h` — **5j**
- [ ] **5.6** Navigation & Pathfinding IA (Recast/Detour, NavMesh dynamique) — `NavMesh.h/.cpp`, `PathfindingSystem.h`, `AINavigationComponent.h` — **5j**

## MODULE 06 — Audio 3D

- [ ] **6.5** Occlusion Sonore (multi-raycast, filtre passe-bas par matériau) — `AudioOcclusion.h/.cpp`, `MaterialAcoustics.h` — **3j**

## MODULE 09 — Network

- [ ] **9.6** Server Meshing (découpage monde en zones, handoff seamless) — `ServerMesh.h/.cpp`, `ZoneServer.h`, `PlayerHandoff.h` — **10j**

## MODULE 10 — Editor

- [ ] **10.3** Overlay Debug Gaze Raycast (ligne 3D, cercle cible, histogramme focus) — `GazeDebugOverlay.h/.cpp`, `DebugRenderer.h` — **2j**
- [ ] **10.5** Sérialisation de Scène (JSON, réflexion statique, format .scene) — `SceneSerializer.h/.cpp`, `ComponentSerializer.h`, `Reflection.h` — **4j**
