#!/usr/bin/env bash
# Télécharge les modèles STT utilisés par config/ai_toolchain.json.
#   whisper_cpp  (ggml-small.bin multilingue, poids OpenAI MIT)  → assets/models/whisper/
#   crisper_whisper (option, POIDS NON-COMMERCIAUX — prototypage) → assets/models/crisper/
# Utilisation : scripts/tools/fetch_models.sh [whisper|all]

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TARGET="${1:-whisper}"

WHISPER_DIR="${ROOT}/assets/models/whisper"
CRISPER_DIR="${ROOT}/assets/models/crisper"
BASE_WHISPER="https://huggingface.co/ggerganov/whisper.cpp/resolve/main"
BASE_CRISPER="https://huggingface.co/drbaph/CrisperWhisper2.0-GGML/resolve/main"
CRISPER_MODEL="ggml-crisperwhisper-small-f16.bin"

mkdir -p "${WHISPER_DIR}" "${CRISPER_DIR}"

case "${TARGET}" in
  whisper)
    echo "> Téléchargement ggml-small.bin (466 Mo, multilingue, FR supporté)..."
    wget -q --show-progress -c -O "${WHISPER_DIR}/ggml-small.bin" \
         "${BASE_WHISPER}/ggml-small.bin"
    echo "> OK : ${WHISPER_DIR}/ggml-small.bin"
    ;;
  crisper)
    echo ""
    echo "!!! POIDS NON-COMMERCIAUX (CC-BY-NC-4.0 / Nyra Research License) !!!"
    echo "    CrisperWhisper 2.0 est INTERDIT dans le build commercial."
    echo "    Réservé aux TESTS/prototypage. Backend prod = whisper.cpp (MIT)."
    echo ""
    echo "> Téléchargement ${CRISPER_MODEL} (+ sidecar metadata)..."
    wget -q --show-progress -c -O "${CRISPER_DIR}/${CRISPER_MODEL}" \
         "${BASE_CRISPER}/${CRISPER_MODEL}"
    wget -q -c -O "${CRISPER_DIR}/${CRISPER_MODEL}.metadata.json" \
         "${BASE_CRISPER}/${CRISPER_MODEL}.metadata.json"
    echo "> OK : ${CRISPER_DIR}/${CRISPER_MODEL} (+ .metadata.json)"
    echo "> CLI requis : scripts/tools/build_crisper_cli.sh"
    ;;
  all)
    "$0" whisper
    "$0" crisper
    ;;
  *)
    echo "Usage: $0 [whisper|all]"
    exit 1
    ;;
esac