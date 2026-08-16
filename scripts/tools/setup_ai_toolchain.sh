#!/usr/bin/env bash
# Setup / mise à jour de l'AI toolchain. Lit config/ai_toolchain.json et :
#   - vérifie le matériel (config/hardware.json)
#   - télécharge les modèles des outils "enable": true
#   - signale les outils disable + les licences à risque
#   - vérifie les dépendances (wget, python3, llama-server, comfy...)
#
# Réutilisable : après toute modification de config/ai_toolchain.json,
# relancez ce script pour propager le changement. Utilisation : scripts/tools/setup_ai_toolchain.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CONFIG="${ROOT}/config/ai_toolchain.json"
HARDWARE="${ROOT}/config/hardware.json"

[ -f "${CONFIG}" ] || { echo "> Config absente : ${CONFIG}"; exit 1; }
[ -f "${HARDWARE}" ] || { echo "> Profil matériel absent : lancez scripts/tools/detect_hardware.sh"; exit 1; }

read_tools() { python3 - "$1" <<'PY'
import json, sys
cfg = json.load(open(sys.argv[1]))
for t in cfg["tools"]:
    print(f"{t['id']}|{t['category']}|{t.get('status','')}|{t.get('enable',False)}|{t['feasibility'].get('recommended_model','')}")
PY
}

echo "══════════════════════════════════════════════"
echo " Synapse Engine — AI Toolchain Setup"
echo "══════════════════════════════════════════════"

# 1) Contraintes matérielles
VRAM="$(python3 -c "import json;print(json.load(open('${HARDWARE}'))['gpus']['primary']['vram_gib'])" 2>/dev/null || echo 8)"
RAM="$(python3 -c "import json;print(json.load(open('${HARDWARE}'))['memory']['total_gib'])" 2>/dev/null || echo 14)"
echo "> Matériel : ${VRAM} Go VRAM, ${RAM} Go RAM"

# 2) Parcours des outils
WARN=0
while IFS='|' read -r id category status enable model; do
    if [ "${enable}" = "True" ]; then
        echo "  [ACTIF ] ${id} (${category}) → ${model:-...}"
        case "${id}" in
          crisper_whisper2)
            [ -f "${ROOT}/assets/models/whisper/ggml-small.bin" ] \
              || "${ROOT}/scripts/tools/fetch_models.sh" whisper
            if grep -q '"primary": true' "${CONFIG}"; then
                echo "    └─ CrisperWhisper (principal) : poids + CLI"
                [ -x "${ROOT}/third_party/CrisperWhisper.cpp/build/bin/crisper-whisper" ] \
                  || "${ROOT}/scripts/tools/build_crisper_cli.sh"
                [ -f "${ROOT}/assets/models/crisper/ggml-crisperwhisper-small-f16.bin" ] \
                  || "${ROOT}/scripts/tools/fetch_models.sh" crisper
            fi
            ;;
          tiered_ecs_gemini) : ;; # code C++, déjà au build
        esac
    else
        echo "  [ARRET ] ${id} (${category}) — enable=false dans ${CONFIG}"
    fi
done < <(read_tools "${CONFIG}")

# 3) Licences sensibles
echo ""
echo "> Rappel licences (config/ai_toolchain.json > license_compliance) :"
grep -n "Non-Commercial Research" "${CONFIG}" >/dev/null \
  && echo "  ⚠  CrisperWhisper 2.0 (poids nyralabs) = NON-COMMERCIAL → prototypage seulement."
grep -n '"usable_in_game": "NON"' "${CONFIG}" >/dev/null \
  && echo "  ⚠  Certains poids ne sont PAS utilisables dans le jeu — vérifiez la table licences."

# 4) Dépendances
MISSING=0
for cmd in wget python3 cmake ninja; do
    command -v "${cmd}" >/dev/null 2>&1 || { echo "> Dépendance manquante : ${cmd}"; MISSING=1; }
done
[ "${MISSING}" = "1" ] && { echo "> Installez les dépendances manquantes puis relancez."; exit 1; }

echo ""
echo "> Terminé. Runtime intégré : STTPipeline (voix) + TieredScheduler (3 niveaux)."
echo "> Doc : docs/ai-toolchain.md — reconfiguration : éditez config/ai_toolchain.json puis relancez ce script."