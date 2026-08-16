#!/usr/bin/env bash
# Construit le CLI CrisperWhisper.cpp (repo Saganaki22, code MIT) pour les
# TESTS / prototypage. Les POIDS restent non-commerciaux (CC-BY-NC-4.0) —
# voir config/ai_toolchain.json. Production = backend whisper.cpp (MIT).
#
# Génère : third_party/CrisperWhisper.cpp/build/bin/crisper-whisper
# Puis     scripts/tools/fetch_models.sh crisper   (poids)
# Et       scripts/tools/setup_ai_toolchain.sh     (validation)
# Utilisation : scripts/tools/build_crisper_cli.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
REPO="${ROOT}/third_party/CrisperWhisper.cpp"

if ! command -v git >/dev/null 2>&1; then
    echo "> git requis."; exit 1
fi

if [ ! -d "${REPO}/.git" ]; then
    echo "> Clone Saganaki22/CrisperWhisper.cpp (code MIT)..."
    git clone --depth 1 https://github.com/Saganaki22/CrisperWhisper.cpp "${REPO}"
else
    echo "> Repo déjà présent : ${REPO}"
fi

cd "${REPO}"

if [ ! -f build/bin/crisper-whisper ]; then
    echo "> Build CPU du CLI (profile balance)..."
    ./scripts/build.sh --cpu --cpu-profile balance
else
    echo "> CLI déjà construit."
fi

BIN="${REPO}/build/bin/crisper-whisper"
if [ ! -x "${BIN}" ]; then
    echo "> Échec du build (voir BUILD_LINUX.md du repo)."
    exit 1
fi

echo "> CLI prêt : ${BIN}"
echo "> Poids :  scripts/tools/fetch_models.sh crisper"
echo "> Test   : ${BIN} -m assets/models/crisper/ggml-crisperwhisper-small-f16.bin -f <wav> --mode verbatim --word-timestamps --json -l fr"