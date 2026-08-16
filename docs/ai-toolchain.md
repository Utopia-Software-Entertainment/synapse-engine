# AI Toolchain — Intégration des outils de la vidéo

Analyse et intégration des 5 outils open-source présentés dans
**"L'IA vient de devenir gratuite (et personne n'en parle)"** — Vision IA
([vidéo](https://youtu.be/Pht0jC4mg40)).

Source de vérité pour la configuration : `config/ai_toolchain.json`.
Profil matériel : `config/hardware.json` (regénérable).

---

## 1. Résumé des 5 outils

| # | Outil | Timestamp vidéo | Catégorie | Statut |
|---|-------|-----------------|-----------|--------|
| 1 | Crisper Whisper 2 | [17:04](https://youtu.be/Pht0jC4mg40?t=1024) | runtime | **Actif** (whisper.cpp prod + CrisperWhisper **tests**) |
| 2 | Wan Animate 2 | [04:18](https://youtu.be/Pht0jC4mg40?t=258) | tooling | Arrêté (enable=false) |
| 3 | MiniMax H3 | [01:16](https://youtu.be/Pht0jC4mg40?t=76) | tooling | Arrêté (enable=false) |
| 4 | Gemini Robotics 2 (archi) | [12:05](https://youtu.be/Pht0jC4mg40?t=725) | runtime | **Actif** (TieredScheduler) |
| 5 | Qwen 3.8 | [06:40](https://youtu.be/Pht0jC4mg40?t=400) | tooling dev | Arrêté (enable=false) |

## 2. Faisabilité matérielle (ce PC : RTX 4060 8 Go, 14 Go RAM)

| Outil | Faisabilité | Raison |
|-------|-------------|--------|
| **STT voix** | ⭐ Excellent | 200M-2B params, CPU uniquement → 0 VRAM, ne dégrade pas le rendu VR 90 FPS |
| **TieredScheduler** | ⭐ Excellent | 100% CPU multi-thread (16 threads) |
| **Qwen 27B** | ❌ Faible | Q4_K_M ≈ 17 Go → dépasse la VRAM (8 Go) et la RAM (14 Go) |
| **Wan Animate 2** | ⚠️ Limité | Full ≈ 33 Go (disque 59 Go) ; Wan 2 Lite envisageable |
| **MiniMax H3** | ⚠️ Limité | 22B fortement quantifié seulement, en batch offline |

> **Recommandation Qwen** : utiliser un **Qwen3.8-8B Q4_K_M (~5 Go)** en local pour
> l'édition de code (voir `scripts/tools/serve_qwen.sh`). Le 27B est réservé à
> une machine 24 Go+ ou un usage cloud occasionnel.

## 3. Table des licences (lecture obligatoire avant usage commercial)

| Composant | Licence | Utilisable en jeu ? |
|-----------|---------|---------------------|
| CrisperWhisper 2.0 (**poids** nyralabs) | Non-Commercial Research | **NON** |
| CrisperWhisper.cpp (**code**) | MIT | OUI (le code), poids non-commerciaux |
| whisper.cpp (ggerganov) | MIT | OUI |
| OpenAI Whisper (**poids**) | MIT | OUI |
| Qwen3.8 | Apache-2.0 | OUI (dev tool) |
| MiniMax H3 | Apache-2.0 annoncé | tooling seulement — **vérifier** |
| Wan Animate 2 | Apache-2.0 annoncé | tooling seulement — **vérifier** |

> **Décision STT** : backend de production = **whisper.cpp + poids OpenAI Whisper (MIT)**.
> CrisperWhisper 2.0 = prototypage uniquement (validations d'incantations en interne).

## 4. Intégration dans le moteur

### 4.1 Voix (source : Crisper Whisper 2) — `src/voice/STT/`

Flux : `MicCapture (16 kHz mono) → STTPipeline → IntentParser/CommandMatcher → EventBus → sorts`

- `STTBackend.h` : interface + `STTResult` (texte + `WordTimestamp` mot-à-mot) + `STTMode`.
- `WhisperBackend.{h,cpp}` : whisper.cpp, timestamps token-level
  (`whisper_full_get_token_data`, `t0`/`t1` ms), mode verbatim, CPU (VRAM libre).
- `CrisperWhisperBackend.{h,cpp}` : bridge CLI **mode TESTS** (prototypage).
- `STTPipeline.{h,cpp}` : orchestrateur, choix du backend via `config/ai_toolchain.json`
  (`STTPipeline::LoadConfigFromToolchain` — `crisperwhisper_cpp.status == "active_for_tests"`).

Latence cible (CPU, modèle small) : **~200-400 ms** pour une commande de 1-2 s.

### CrisperWhisper en backend principal

CrisperWhisper est le backend **principal** pour les tests du gameplay (`"primary": true` dans
`config/ai_toolchain.json`) ; whisper.cpp sert de repli production.

```bash
scripts/tools/install_prereqs.sh  # prérequis système (une fois)
scripts/tools/build_crisper_cli.sh   # CLI (code MIT) → third_party/CrisperWhisper.cpp/build/bin/
scripts/tools/fetch_models.sh crisper # poids small-f16 + sidecar → assets/models/crisper/
scripts/tools/setup_ai_toolchain.sh   # validation / téléchargements
```

Test manuel (schéma JSON : `text`, `words[].word|start|end`) :

```bash
third_party/CrisperWhisper.cpp/build/bin/crisper-whisper \
  -m assets/models/crisper/ggml-crisperwhisper-small-f16.bin \
  -f <audio.wav> --mode verbatim --word-timestamps --json -l fr
```

> ⚠ **Jamais dans le build commercial** : les poids CrisperWhisper 2.0 sont sous
> CC-BY-NC-4.0. Passer `"primary": false` sur `crisperwhisper_cpp` (et `"primary": true`
> sur `whisper_cpp`) dans `config/ai_toolchain.json` pour repasser en whisper.cpp pur.

### 4.2 Boucle VR hiérarchisée (source : Gemini Robotics 2) — `src/core/Scheduler/`

`TieredScheduler` : trois niveaux inspirés de Gemini Robotics 2.

- **Réflexe (90 Hz, synchrone, thread principal)** : eye-tracking, EMG (blink/mâchoire),
  input, gaze raycast — avant le rendu.
- **Action (60 Hz, synchrone)** : animation, gameplay.
- **Cerveau (async taskflow, 6 threads)** : IA PNJ, dialogues, pathfinding — les
  calculs longs ne bloquent plus la boucle VR.

## 5. Scripts `scripts/tools/`

| Script | Rôle |
|--------|------|
| `detect_hardware.sh` | régénère `config/hardware.json` |
| `fetch_models.sh [whisper\|all]` | télécharge ggml-small.bin (whisper) et option CrisperWhisper |
| `build_crisper_cli.sh` | clone + build CPU du CLI CrisperWhisper.cpp (code MIT) |
| `serve_qwen.sh [--daemon]` | llama-server pour Qwen3.8-8B (outil dev) |
| `run_wan_animate.sh` | lance ComfyUI pour Wan Animate 2 / Wan 2 Lite (tooling) |
| `run_minimax_h3.sh` | batch MiniMax H3 (tooling) |
| `setup_ai_toolchain.sh` | lit `config/ai_toolchain.json`, télécharge les modèles des outils actifs, vérifie dépendances et licences |

## 6. Procédure de mise à jour

1. Éditer `config/ai_toolchain.json` (flags `enable`, paths, modèles, fréquences).
2. Relancer `scripts/tools/setup_ai_toolchain.sh` (propagation).
3. Pour le matériel changé : `scripts/tools/detect_hardware.sh` puis re-setup.
4. Vérifier : `cmake --build build --target synapse_core synapse_voice -j8`.