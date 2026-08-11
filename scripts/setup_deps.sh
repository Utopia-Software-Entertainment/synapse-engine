#!/usr/bin/env bash
set -euo pipefail

# ── Synapse Engine — External dependency installer ──
# Downloads Steam Audio SDK + liblsl into third_party/

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
THIRD_DIR="$PROJECT_DIR/third_party"

echo "[*] Installing non-system dependencies to $THIRD_DIR"

mkdir -p "$THIRD_DIR"

# ============================================================
# Steam Audio SDK
# ============================================================

STEAM_AUDIO_VERSION="4.5.1"
STEAM_AUDIO_DIR="$THIRD_DIR/steamaudio"
STEAM_AUDIO_URL="https://github.com/ValveSoftware/steam-audio/releases/download/v${STEAM_AUDIO_VERSION}/steamaudio_${STEAM_AUDIO_VERSION}_linux.tar.gz"

if [ ! -d "$STEAM_AUDIO_DIR" ]; then
    echo "[*] Downloading Steam Audio SDK v${STEAM_AUDIO_VERSION}..."
    mkdir -p /tmp/steamaudio
    wget -q --show-progress -O /tmp/steamaudio/steamaudio.tar.gz "$STEAM_AUDIO_URL"
    tar -xzf /tmp/steamaudio/steamaudio.tar.gz -C /tmp/steamaudio
    # The tarball extracts to steam-audio-sdk-<version>/
    EXTRACTED=$(find /tmp/steamaudio -maxdepth 1 -type d -name "steam-audio-sdk*" | head -1)
    if [ -n "$EXTRACTED" ]; then
        mv "$EXTRACTED" "$STEAM_AUDIO_DIR"
    else
        # Try common extraction name
        mkdir -p "$STEAM_AUDIO_DIR"
        cp -r /tmp/steamaudio/* "$STEAM_AUDIO_DIR/" 2>/dev/null || true
    fi
    rm -rf /tmp/steamaudio
    echo "[✓] Steam Audio SDK installed at $STEAM_AUDIO_DIR"
else
    echo "[✓] Steam Audio SDK already present at $STEAM_AUDIO_DIR"
fi

# ============================================================
# liblsl (Lab Streaming Layer)
# ============================================================

LSL_DIR="$THIRD_DIR/liblsl"
LSL_BUILD_DIR="$LSL_DIR/build"

if [ ! -d "$LSL_DIR/lib" ]; then
    echo "[*] Cloning liblsl..."
    git clone --depth 1 --branch v1.16.2 https://github.com/sccn/liblsl.git "$LSL_DIR" 2>/dev/null || {
        echo "[!] liblsl already cloned, rebuilding..."
    }

    echo "[*] Building liblsl..."
    mkdir -p "$LSL_BUILD_DIR"
    cmake -S "$LSL_DIR" -B "$LSL_BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF
    cmake --build "$LSL_BUILD_DIR" -j "$(nproc)"

    # Install to local prefix
    cmake --install "$LSL_BUILD_DIR" --prefix "$LSL_DIR"
    echo "[✓] liblsl installed at $LSL_DIR"
else
    echo "[✓] liblsl already present at $LSL_DIR"
fi

# ============================================================
# Steam Audio — update package config (NO install)
# ============================================================
echo ""
echo "[✓] All done!"
echo ""
echo "Steam Audio headers:  $STEAM_AUDIO_DIR/include"
echo "Steam Audio libs:     $STEAM_AUDIO_DIR/lib/linux-x64"
echo "liblsl headers:       $LSL_DIR/include"
echo "liblsl libs:          $LSL_DIR/lib"
echo ""
echo "Add these paths if CMake doesn't find them automatically:"
echo "  -DCMAKE_PREFIX_PATH=$LSL_DIR"
echo "  -DSTEAM_AUDIO_SDK=$STEAM_AUDIO_DIR"
