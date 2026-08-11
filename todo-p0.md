# 🔴 SYNAPSE ENGINE — TODO P0 BLOQUANT

> **Rien ne peut avancer sans ces tâches.**
> Durée totale estimée : **~93 jours**

---

## MODULE 01 — Core & ECS (Phase 1)

- [ ] **1.1** Système de Logs (spdlog/quill) — `Logger.h/.cpp` — **2j**
- [ ] **1.2** Allocateurs Mémoire Custom (Arena/Pool/Stack) — `MemoryArena.h`, `PoolAllocator.h`, `StackAllocator.h` — **3j**
- [ ] **1.3** Types Fondamentaux & UUID — `Types.h`, `UUID.h/.cpp`, `Handle.h` — **1j**
- [ ] **1.4** Bibliothèque Mathématique SIMD (Vec2/3/4, Mat4, Quat, Ray, Frustum) — `Math.h`, `Vector.h`, `Matrix.h`, `Quaternion.h`, `Ray.h`, `Frustum.h` — **2j**
- [ ] **1.5** ECS Core — Registre d'Entités (EnTT, Registry, Views, Components) — `Registry.h`, `Entity.h`, `Component.h` — **4j**
- [ ] **1.6** Scheduler de Systèmes (ordre d'exécution, task queue) — `SystemScheduler.h/.cpp`, `SystemGroup.h` — **3j**
- [ ] **1.8** Timer & Delta Time (haute précision, fixed/variable timestep) — `Timer.h/.cpp`, `FrameTimer.h`, `FixedTimestep.h` — **1j**
- [ ] **1.9** Game Loop Principale Multi-threadée (3 threads, Double Buffer, 11ms budget) — `Engine.h/.cpp`, `MainLoop.h`, `ThreadPool.h`, `DoubleBuffer.h` — **5j**

## MODULE 02 — Platform & Windowing (Phase 1)

- [ ] **2.1** Abstraction OS (HAL) — `Platform.h`, `WindowsPlatform.cpp`, `LinuxPlatform.cpp` — **2j**
- [ ] **2.2** Gestion Fenêtre GLFW (VkSurface, mirror VR) — `Window.h/.cpp`, `WindowEvents.h` — **2j**

## MODULE 03 — Renderer Vulkan (Phase 2)

- [ ] **3.1** Init Vulkan — Instance & Device (VkInstance, VkDevice, queues, layers, multiview) — `VulkanContext.h/.cpp`, `VulkanDevice.h` — **4j**
- [ ] **3.2** Swapchain & Surface (double buffering, swapchains stéréo gauche/droite) — `Swapchain.h/.cpp`, `VulkanSurface.h`, `XRSwapchain.h` — **3j**
- [ ] **3.3** Pipeline Graphique & Shaders (GLSL → SPIR-V, hot-reload) — `Pipeline.h/.cpp`, `Shader.h/.cpp`, `ShaderCompiler.h` — **5j**
- [ ] **3.4** Render Pass & Framebuffers (Main Pass, Resolve Pass, MSAA) — `RenderPass.h/.cpp`, `Framebuffer.h`, `RenderGraph.h` — **3j**
- [ ] **3.5** Gestion Mémoire GPU (VMA) — `GPUAllocator.h/.cpp`, `VulkanBuffer.h`, `VulkanImage.h` — **2j**
- [ ] **3.12** Stereo VR Rendering — Double Passe (Single-Pass Instanced, VK_KHR_multiview) — `StereoRenderer.h/.cpp`, `EyeMatrices.h`, `StereoFramebuffer.h` — **5j**
- [ ] **3.13** Foveated Rendering Dynamique (rendu fovéal, économie GPU 50-70%) — `FoveatedPass.h/.cpp`, `FoveationMap.h` — **6j**

## MODULE 04 — VR & Eye-Tracking (Phase 2)

- [ ] **4.1** Initialisation Session OpenXR (XrInstance, XrSession, cycle de vie) — `XRSession.h/.cpp`, `XRInstance.h`, `XRLifecycle.h` — **4j**
- [ ] **4.2** Head Tracking 6DoF (xrLocateSpace, position/orientation quaternion) — `XRHeadTracker.h/.cpp`, `XRPose.h` — **2j**
- [ ] **4.3** Eye-Tracking — Vecteur de Regard (XR_EXT_eye_gaze, calibration, Kalman filter) — `XREyeTracker.h/.cpp`, `GazeFilter.h`, `KalmanFilter.h` — **4j**
- [ ] **4.4** Gaze Raycast System (LE CŒUR DU GAMEPLAY — 120Hz, latence <3ms) — `GazeRaycastSystem.h/.cpp`, `GazeTargetComponent.h`, `GazeFocusEvent.h` — **5j**
- [ ] **4.5** Matrices de Projection Stéréo (View/Projection par œil, UBO) — `XRProjection.h/.cpp`, `ViewProjectionUBO.h` — **2j**
- [ ] **4.6** Mode Allongé — Recalibration Spatiale (Pitch -90°, horizon virtuel) — `RecumbentMode.h/.cpp`, `PostureCalibration.h` — **3j**
- [ ] **4.8** Composant ECS EyeGazeComponent (pont entre VR et ECS) — `EyeGazeComponent.h`, `EyeGazeUpdateSystem.h` — **1j**

## MODULE 05 — Physics (Phase 2)

- [ ] **5.1** Init Monde Physique Jolt (JobSystem multi-threadé, layers) — `PhysicsWorld.h/.cpp`, `PhysicsLayers.h`, `JoltJobSystem.h` — **3j**
- [ ] **5.2** Rigid Bodies (Box, Capsule, Sphere, Mesh + PhysicsBodyComponent ECS) — `RigidBody.h/.cpp`, `CollisionShape.h`, `PhysicsBodyComponent.h` — **3j**

## MODULE 06 — Audio 3D (Phase 3)

- [ ] **6.1** Init Moteur Audio (FMOD/OpenAL abstraction) — `AudioEngine.h/.cpp`, `FMODAudioEngine.cpp`, `OpenALEngine.cpp` — **3j**
- [ ] **6.2** Sources Audio 3D Positionnées (AudioEmitterComponent, listener tracking) — `AudioSource.h/.cpp`, `AudioEmitterComponent.h`, `AudioListenerSystem.h` — **3j**
- [ ] **6.3** HRTF — Spatialisation Réaliste (MIT KEMAR, convolution GPU) — `HRTFProcessor.h/.cpp`, `HRTFDatabase.h`, `ConvolutionReverb.h` — **5j**

## MODULE 07 — Neural Input (Phase 3)

- [ ] **7.1** Connexion LSL (Lab Streaming Layer, inlet 500Hz) — `LSLReceiver.h/.cpp`, `LSLStreamInfo.h` — **3j**
- [ ] **7.2** Parser EEG Brut (Cyton Board, filtre Notch 50/60Hz) — `EEGParser.h/.cpp`, `NotchFilter.h`, `EEGChannelMap.h` — **3j**
- [ ] **7.3** Détecteur Serrage de Dents (EMG Jaw, seuillage adaptatif, <5ms) — `JawClenchDetector.h/.cpp`, `EMGThresholder.h`, `AdaptiveBaseline.h` — **4j**
- [ ] **7.6** Filtre Anti-bruit Neuronal (Butterworth, CAR, ICA simplifié) — `NeuralFilter.h/.cpp`, `ButterworthFilter.h`, `CARFilter.h`, `ArtifactRejector.h` — **4j**
- [ ] **7.7** Composant ECS NeuralInputComponent (centralise toutes les entrées bio) — `NeuralInputComponent.h`, `BioInputBridge.h` — **1j**
- [ ] **7.8** Bio-to-Action Mapping System (tableau mapping, machine d'état combos) — `BioInputMappingSystem.h/.cpp`, `ActionMapping.h`, `BioComboDetector.h` — **4j**

## MODULE 08 — AI Vocale (Phase 3)

- [ ] **8.1** Intégration Whisper.cpp (modèle small/tiny, GPU CUDA/Vulkan, offline) — `VoiceRecognizer.h/.cpp`, `WhisperContext.h`, `ModelLoader.h` — **4j**
- [ ] **8.2** Capture Micro Temps Réel (PortAudio, VAD, réduction bruit) — `MicCapture.h/.cpp`, `VADDetector.h`, `NoiseSuppressor.h` — **3j**
- [ ] **8.3** Pipeline STT Asynchrone (thread séparé, circular buffer, transcription queue) — `STTPipeline.h/.cpp`, `AudioCircularBuffer.h`, `TranscriptionQueue.h` — **3j**

## MODULE 09 — Network (Phase 4)

- [ ] **9.1** Couche Transport UDP (reliability sélective, ordering, congestion control) — `NetworkSocket.h/.cpp`, `PacketProtocol.h`, `ReliableUDP.h` — **5j**
- [ ] **9.2** Sérialisation Binaire (compression delta, bit packing) — `Serializer.h/.cpp`, `BitStream.h`, `DeltaEncoder.h` — **4j**
