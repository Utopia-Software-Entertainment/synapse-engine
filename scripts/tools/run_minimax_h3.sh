#!/usr/bin/env bash
# MiniMax H3 — world model vidéo+audio (tooling offline occasionnel).
# Génère du gameplay visuel de référence / prototypes de physique.
# Config : config/ai_toolchain.json > tools[2]
#
# Garde-fous de cette machine (8 Go VRAM) :
#   - MiniMax-H3 22B fortement quantifié seulement, et en mode BATCH offline.
#   - Jamais en runtime pendant la session VR (saccades garanties).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${ROOT}/docs/prototypes"
mkdir -p "${OUT}"

# Adaptez au runner que vous installez (HF diffusers / ComfyUI / demo officielle).
RUNNER="${RUNNER:-}"
GGUF="${GGUF:-}"

if [ -n "${GGUF}" ] && [ -f "${GGUF}" ]; then
    echo "> Exécution batch H3 sur ${GGUF}..."
    exec llama-cli -m "${GGUF}" --no-display-prompt < /dev/stdin
fi

if [ -n "${RUNNER}" ]; then
    exec "${RUNNER}"
fi

cat <<'EOF'
> Runner H3 non configuré. Options pour ce PC (8 Go VRAM) :

  1) Vérifiez la licence exacte avant usage commercial (config ai_toolchain.json).
  2) 22B quantifié (Q2/Q3) : lancer via diffusers / ComfyUI en mode offline BATCH.
  3) Machine dédiée (16 Go+) : H3 33B confortable.

  Sortie attendue : vidéos/audio de référence dans docs/prototypes/.

  Exemple :
    RUNNER="python3 ${ROOT}/third_party/minimax_h3/demo.py" \
    scripts/tools/run_minimax_h3.sh
EOF
exit 1