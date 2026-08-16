#!/usr/bin/env bash
# Wan Animate 2 — MoCap virtuel offline (tooling UNIQUEMENT, hors runtime).
# Génère des banques d'animations transférées (boss non-humains) consommées
# par src/physics/IK + le pipeline d'animation. Config : config/ai_toolchain.json > tools[1]
#
# Garde-fous de cette machine (8 Go VRAM / 59 Go disque) :
#   - Wan 2 Lite (<1s) via ComfyUI : OK en INT8 / hors-mémoire (vram + offload).
#   - Wan 2 full (~33 Go) : à faire sur une machine dédiée, PAS ici.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${ROOT}/assets/animations"
mkdir -p "${OUT}"

if ! command -v comfy >/dev/null 2>&1 && [ ! -d "${ROOT}/third_party/ComfyUI" ]; then
    echo "> ComfyUI introuvable."
    echo "  Option A : 'comfy' (pipx install comfyui) puis relancez."
    echo "  Option B : clonez dans third_party/ComfyUI et lancez avec --listen 127.0.0.1:8188"
    echo "  Workflow JSON (Wan 2 Lite, driver video + transfert) :"
    echo "    https://comfyanonymous.github.io/ComfyUI_examples/video/"
    echo "  Sortie attendue : clips glTF/GLB exportés vers ${OUT}/"
    exit 1
fi

echo "> Lancement ComfyUI (workflow Wan Animate 2 / Wan 2 Lite)..."
echo "> Sortie  : ${OUT}/"
if [ -d "${ROOT}/third_party/ComfyUI" ]; then
    exec python3 "${ROOT}/third_party/ComfyUI/main.py" --listen 127.0.0.1:8188
else
    exec comfy launch
fi