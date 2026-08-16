#!/usr/bin/env bash
# Lance llama-server (llama.cpp) pour l'agent de codage local Qwen3.8.
# Config lue : config/ai_toolchain.json > tools[4]
#
# IMPORTANT (calibrage 8 Go VRAM) :
#   Qwen3.8-27B Q4_K_M = ~17 Go → IMPOSSIBLE sur cette machine.
#   Ce script cible Qwen3.8-8B Q4_K_M (~5 Go) pour l'édition de code.
#   Le 27B (Q2/Q3 + offload CPU) n'est pas servi par ce script.
#
# Utilisation :
#   scripts/tools/serve_qwen.sh            # serveur en avant-plan
#   scripts/tools/serve_qwen.sh --daemon   # serveur en arrière-plan

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PORT="$(python3 -c "import json;print(json.load(open('${ROOT}/config/ai_toolchain.json'))['tools'][4].get('llama_server_port',8080))" 2>/dev/null || echo 8080)"
GGUF="${ROOT}/$(python3 -c "import json;print(json.load(open('${ROOT}/config/ai_toolchain.json'))['tools'][4].get('gguf_path',''))" 2>/dev/null || echo 'models/qwen3.8/Qwen3.8-8B-Q4_K_M.gguf')"

if ! command -v llama-server >/dev/null 2>&1; then
    echo "> llama-server introuvable. Installation :"
    echo "    brew install llama.cpp   (macOS)  |  pip install llama-cpp-python[server]  (Python)"
    echo "    ou https://github.com/ggml-org/llama.cpp (releases)"
    exit 1
fi

if [ ! -f "${GGUF}" ]; then
    echo "> Modèle absent : ${GGUF}"
    echo "> Téléchargez Qwen3.8-8B (ou 7B/8B Q4_K_M) et placez-le à ce chemin,"
    echo "> ou modifiez 'gguf_path' dans config/ai_toolchain.json."
    exit 1
fi

echo "> Modèle  : ${GGUF}"
echo "> Port    : ${PORT}"
echo "> VRAM    : -ngl 24 (laisser de la place au rendu VR)"

ARGS=(--model "${GGUF}" --port "${PORT}" --host 127.0.0.1 -ngl 24 -c 4096 --no-warmup)

if [ "${1:-}" = "--daemon" ]; then
    nohup llama-server "${ARGS[@]}" >/tmp/qwen-server.log 2>&1 &
    echo "> Serveur lancé en arrière-plan (pid $!). Logs : /tmp/qwen-server.log"
    echo "> API OpenAI-compatible : http://127.0.0.1:${PORT}/v1"
else
    echo "> Ctrl+C pour arrêter."
    exec llama-server "${ARGS[@]}"
fi