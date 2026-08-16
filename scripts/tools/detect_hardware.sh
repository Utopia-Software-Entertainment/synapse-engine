#!/usr/bin/env bash
# Régénère config/hardware.json à partir des outils système (lscpu, free, nvidia-smi...).
# Utilisation : scripts/tools/detect_hardware.sh

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT="${ROOT}/config/hardware.json"

echo "> Detection hardware..."

CPU_MODEL="$(lscpu | awk -F': ' '/Model name/{sub(/^ *@.*/, "", $2); print $2; exit}')"
CPU_CORES="$(nproc)"
CPU_MAX_MHZ="$(lscpu | awk -F': +' '/CPU max MHz/{print int($2); exit}')"

TOTAL_GIB="$(free -g | awk '/Mem:/{print $2}')"
AVAIL_GIB="$(free -g | awk '/Mem:/{print $7}')"
SWAP_GIB="$(free -g | awk '/Swap:/{print $2}')"

if command -v nvidia-smi >/dev/null 2>&1; then
    GPU_NAME="$(nvidia-smi --query-gpu=name --format=csv,noheader,nounits | head -1)"
    VRAM_GIB="$(nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits | head -1 | awk '{print int($1/1024)}')"
    DRIVER="$(nvidia-smi --query-gpu=driver_version --format=csv,noheader,nounits | head -1)"
    CUDA="$(nvidia-smi | awk '/CUDA Version/{print $9; exit}')"
else
    GPU_NAME="(non détecté)"
    VRAM_GIB=0
    DRIVER="-"
    CUDA="-"
fi

VULKAN="$(vulkaninfo --summary 2>/dev/null | awk -F': ' '/apiVersion/{print $2; exit}')" || VULKAN="(vulkaninfo absent)"

DISK_TOTAL_GIB="$(df -BG --output=size / | awk 'NR==2{gsub("G","",$1); print $1}')"
DISK_FREE_GIB="$(df -BG --output=avail / | awk 'NR==2{gsub("G","",$1); print $1}')"

DISTRO="$(awk -F'"' '/PRETTY_NAME/{print $2}' /etc/os-release 2>/dev/null || echo "Linux")"
KERNEL="$(uname -r)"

cat > "${OUT}" <<EOF
{
  "profile_name": "DevLaptop1",
  "detected_on": "$(date +%F)",
  "note": "Profil matériel détecté localement. Regénérable avec scripts/tools/detect_hardware.sh. Utilisé par config/ai_toolchain.json pour décider quels outils/quantifications sont faisables.",

  "cpu": {
    "model": "${CPU_MODEL}",
    "cores": ${CPU_CORES},
    "threads": ${CPU_CORES},
    "max_freq_mhz": ${CPU_MAX_MHZ}
  },

  "memory": {
    "total_gib": ${TOTAL_GIB},
    "available_at_detection_gib": ${AVAIL_GIB},
    "swap_gib": ${SWAP_GIB}
  },

  "gpus": {
    "primary": {
      "name": "${GPU_NAME}",
      "vram_gib": ${VRAM_GIB},
      "driver": "${DRIVER}",
      "cuda_version": "${CUDA}",
      "vulkan_api": "${VULKAN}",
      "role": "VR rendering + inference occasionnelle"
    }
  },

  "disk": {
    "root": {
      "mount": "/",
      "total_gib": ${DISK_TOTAL_GIB},
      "free_gib": ${DISK_FREE_GIB}
    }
  },

  "os": {
    "distro": "${DISTRO}",
    "kernel": "${KERNEL}"
  }
}
EOF

echo "> hardware.json écrit dans ${OUT}"
echo "  GPU  : ${GPU_NAME} (${VRAM_GIB} Go, driver ${DRIVER}, CUDA ${CUDA})"
echo "  CPU  : ${CPU_MODEL} (${CPU_CORES} threads, ${CPU_MAX_MHZ} MHz)"
echo "  RAM  : ${TOTAL_GIB} Go (${AVAIL_GIB} Go dispo)  |  Disque : ${DISK_FREE_GIB} Go libres"