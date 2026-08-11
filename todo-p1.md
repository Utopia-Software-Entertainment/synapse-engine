# 🟠 SYNAPSE ENGINE — TODO P1 IMPORTANT

> **Tâches prioritaires du sprint courant.**
> Durée totale estimée : **~74 jours**

---

## MODULE 01 — Core & ECS

- [ ] **1.7** Event Bus Asynchrone (Observer thread-safe, GameEvents) — `EventBus.h/.cpp`, `EventQueue.h`, `Events/GameEvents.h` — **3j**

## MODULE 03 — Renderer Vulkan (Phase 2)

- [ ] **3.6** Chargement de Meshes (glTF 2.0, fastgltf, async) — `MeshLoader.h/.cpp`, `Mesh.h`, `MeshCache.h`, `AsyncAssetLoader.h` — **4j**
- [ ] **3.7** Gestion des Textures (KTX2, BCn/ASTC, streaming mips progressif) — `Texture.h/.cpp`, `TextureCache.h`, `TextureStreamer.h` — **3j**
- [ ] **3.8** Descriptors & Uniform Buffers (DescriptorManager, pools pré-alloués) — `DescriptorManager.h/.cpp`, `DescriptorSetLayout.h`, `UniformBuffer.h` — **3j**
- [ ] **3.9** Forward+ Rendering Pipeline (Light Culling, Compute Shader, centaines lumières) — `ForwardPlusRenderer.h/.cpp`, `LightCullingPass.h`, `LightGrid.h` — **6j**
- [ ] **3.10** Occlusion & Frustum Culling (Hi-Z, 2 frustums VR, 60-90% draw calls éliminés) — `CullingSystem.h/.cpp`, `FrustumCuller.h`, `OcclusionQueries.h` — **4j**
- [ ] **3.15** HAL — Abstraction du Renderer (RenderAPI.h, Pimpl) — `RenderAPI.h`, `VulkanRenderAPI.cpp`, `RenderCommand.h` — **3j**

## MODULE 05 — Physics (Phase 2)

- [ ] **5.3** Collision Detection Continue (CCD) — `CollisionSystem.h/.cpp`, `CCDSettings.h` — **2j**
- [ ] **5.4** Physics Queries (RayCast, ShapeCast, OverlapTest) — `PhysicsQuery.h/.cpp`, `RaycastResult.h`, `OverlapResult.h` — **2j**

## MODULE 06 — Audio 3D (Phase 3)

- [ ] **6.4** Effet Doppler (variation fréquence selon vélocité relative) — `DopplerSystem.h/.cpp` — **1j**
- [ ] **6.6** Composant ECS AudioEmitterComponent (ID son, état, volume, rayon) — `AudioEmitterComponent.h`, `AudioSystem.h/.cpp` — **1j**

## MODULE 07 — Neural Input (Phase 3)

- [ ] **7.4** Analyseur Focus Mental (FFT glissante, ondes α/β, niveau 0.0-1.0) — `MentalFocusAnalyzer.h/.cpp`, `FFTProcessor.h`, `SpectralBandAnalyzer.h` — **5j**
- [ ] **7.5** Détecteur Clignement Volontaire (SVM, EOG, faux-positifs <1%) — `BlinkDetector.h/.cpp`, `EOGClassifier.h` — **4j**

## MODULE 08 — AI Vocale (Phase 3)

- [ ] **8.4** Intent Parser — Texte vers Action (matching règles + fuzzy, voice_commands.json) — `IntentParser.h/.cpp`, `CommandMatcher.h` — **4j**
- [ ] **8.5** VoiceCommandSystem ECS (vérification conditions, anti-spam, GameActionEvents) — `VoiceCommandSystem.h/.cpp`, `VoiceCommandComponent.h` — **2j**

## MODULE 09 — Network (Phase 4)

- [ ] **9.3** Prédiction Client (input queue, simulation locale immédiate) — `ClientPrediction.h/.cpp`, `InputQueue.h`, `PredictedState.h` — **5j**
- [ ] **9.4** Réconciliation Serveur (Rollback, smooth correction, snap interpolation) — `ServerReconciliation.h/.cpp`, `StateSnapshot.h`, `SnapInterpolation.h` — **4j**
- [ ] **9.5** Réplication d'État ECS (DirtyFlagSystem, Interest Management) — `ECSReplication.h/.cpp`, `DirtyFlagSystem.h`, `InterestManager.h` — **6j**
- [ ] **9.7** IA Serveur — Boss State Machine (HSM, LycaonAI, AegisNullAI) — `ServerAISystem.h/.cpp`, `BossStateMachine.h`, `AI/LycaonAI.h`, `AI/AegisNullAI.h` — **8j**

## MODULE 10 — Editor (Phase 5)

- [ ] **10.1** Layer ImGui dans Vulkan (overlay, backends Vulkan+GLFW, hot-reload) — `EditorLayer.h/.cpp`, `ImGuiVulkanBackend.h`, `ImGuiGLFWBackend.h` — **4j**
- [ ] **10.2** Inspecteur d'Entités ECS (liste temps réel, modifiable, colorisation dirty) — `EntityInspector.h/.cpp`, `ComponentRenderer.h` — **4j**
- [ ] **10.4** Console de Logs Temps Réel (filtrage, couleurs, recherche, fichier rotatif) — `LogConsole.h/.cpp`, `LogFilter.h` — **2j**
