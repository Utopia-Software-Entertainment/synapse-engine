#!/usr/bin/env bash
# Installe les prérequis système pour construire / contribuer au Synapse Engine.
# Cible : Ubuntu/Debian (apt) — Fedora (dnf), Arch (pacman), macOS (brew) couverts.
# Idempotent : relançable sans risque. Utilisation : scripts/tools/install_prereqs.sh
#
# Ce que ça installe : compilateur C++23 (g++-13+/clang-17+), CMake, Ninja, ccache,
# Vulkan SDK (loader + validation + glslc), OpenXR SDK, OpenAL (audio), libs USB/HID
# (neural), dépendances GLFW/X11/Wayland.

set -euo pipefail

detect_pkgmgr() {
    if command -v apt-get >/dev/null 2>&1; then echo "apt"
    elif command -v dnf >/dev/null 2>&1; then echo "dnf"
    elif command -v pacman >/dev/null 2>&1; then echo "pacman"
    elif command -v brew >/dev/null 2>&1; then echo "brew"
    else echo "unknown"; fi
}

PKGMGR="$(detect_pkgmgr)"

install_apt() {
    sudo apt-get update
    sudo apt-get install -y --no-install-recommends \
        build-essential g++ cmake ninja-build git ccache pkg-config \
        python3 python3-pip wget curl \
        libvulkan-dev vulkan-tools vulkan-validationlayers glslc \
        libopenxr-dev \
        libopenal-dev \
        libusb-1.0-0-dev libhidapi-dev \
        libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
        libxi-dev libwayland-dev libxkbcommon-dev xorg-dev
    # Le g++ par défaut d'Ubuntu 24.04 = g++-13 (C++23 OK).
    echo "> Vérification C++23..."
    g++ -std=c++23 -x c++ -o /tmp/cxx23check - <<'EOF'
int main() { return 0; }
EOF
    rm -f /tmp/cxx23check
}

install_dnf() {
    sudo dnf groupinstall -y "Development Tools"
    sudo dnf install -y cmake ninja-build git ccache pkgconf-pkg-config \
        python3 python3-pip wget curl \
        vulkan-headers vulkan-loader vulkan-tools vulkan-validation-layers glslc \
        openxr-devel openal-soft-devel \
        libusbx-devel hidapi-devel \
        libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel \
        libXi-devel libwayland-devel libxkbcommon-devel
}

install_pacman() {
    sudo pacman -S --needed --noconfirm base-devel cmake ninja git ccache \
        python python-pip wget curl \
        vulkan-headers vulkan-icd-loader vulkan-tools vulkan-validation-layers glslc \
        openxr openal \
        libusb hidapi \
        libx11 libxrandr libxinerama libxcursor libxi wayland libxkbcommon
}

install_brew() {
    brew install cmake ninja git ccache pkg-config python wget curl
    brew install vulkan-headers vulkan-loader vulkan-validationlayers glslang
    brew install openxr openal-soft
    brew install libusb hidapi
    brew install libx11 libxrandr libxinerama libxcursor libxi libxkbcommon wayland
}

case "${PKGMGR}" in
  apt)    echo "> Détection : apt (Ubuntu/Debian)" ; install_apt ;;
  dnf)    echo "> Détection : dnf (Fedora)"       ; install_dnf ;;
  pacman) echo "> Détection : pacman (Arch)"      ; install_pacman ;;
  brew)   echo "> Détection : brew (macOS)"       ; install_brew ;;
  *)
    echo "> Gestionnaire de paquets non reconnu."
    echo "> Installez manuellement les paquets listés dans SETUP.md."
    exit 1
    ;;
esac

echo ""
echo "══════════════════════════════════════════════"
echo " Prérequis installés ✓"
echo "══════════════════════════════════════════════"
echo " Étape suivante :"
echo "   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug"
echo "   cmake --build build -j"
echo "   scripts/tools/setup_ai_toolchain.sh   (outils IA / modèles)"
echo ""
echo " OPTIONNEL — accélération CUDA pour whisper.cpp (backends GPU) :"
echo "   Installez le CUDA Toolkit 13 via le dépôt NVIDIA, puis rebuild."
echo ""
if command -v nvidia-smi >/dev/null 2>&1; then
    echo " GPU NVIDIA détecté : $(nvidia-smi --query-gpu=name --format=csv,noheader | head -1)"
else
    echo " Aucun GPU NVIDIA détecté (backend CUDA indisponible — CPU OK pour Whisper)."
fi