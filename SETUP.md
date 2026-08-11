# Synapse Engine — System Dependencies

## Ubuntu / Debian (22.04+)

```bash
# Build tools
sudo apt install build-essential cmake ninja-build git

# C++23 compiler (g++-13+ or clang-17+)
sudo apt install g++-13  # or: clang-17 lld-17

# Vulkan SDK (headers + loader + tools + glslc)
sudo apt install libvulkan-dev vulkan-validationlayers glslc

# OpenXR SDK
sudo apt install libopenxr-dev

# OpenAL Soft (3D audio)
sudo apt install libopenal-dev

# BrainFlow (neural) — USB + HID for OpenBCI
sudo apt install libusb-1.0-0-dev libhidapi-dev

# LSL (lab streaming layer, for BCI)
sudo apt install liblsl-dev

# Build essentials for third-party libs
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
                 libxi-dev libwayland-dev libxkbcommon-dev
```

## Arch Linux

```bash
sudo pacman -S base-devel cmake ninja git \
    gcc13  # or: clang17 lld17 \
    vulkan-headers vulkan-loader vulkan-validation-layers glslc \
    openxr openal \
    libusb hidapi liblsl \
    libx11 libxrandr libxinerama libxcursor libxi wayland libxkbcommon
```

## Fedora (39+)

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake ninja-build git \
    gcc13-c++  # or: clang17 lld17 \
    vulkan-headers vulkan-loader vulkan-validation-layers glslc \
    openxr-devel openal-soft-devel \
    libusbx-devel hidapi-devel \
    libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel \
    libXi-devel libwayland-devel libxkbcommon-devel
```

## macOS (Sonnoma+ with Homebrew)

```bash
brew install cmake ninja git gcc@13  # or: llvm@17
brew install vulkan-headers vulkan-loader vulkan-validationlayers glslang
brew install openxr openal-soft libusb hidapi
```

## Windows (with vcpkg)

```powershell
vcpkg install vulkan openxr openal-soft libusb hidapi
```

## Manual / SDK installs

### Vulkan SDK (if distro packages are outdated)
Download from: https://vulkan.lunarg.com/sdk/home
Extract and set: `export VULKAN_SDK=/path/to/sdk`

### OpenXR SDK (if not packaged)
```bash
git clone https://github.com/KhronosGroup/OpenXR-SDK.git
cd OpenXR-SDK && cmake -B build && cmake --build build --target install
```

### LSL (if not packaged)
```bash
git clone https://github.com/sccn/liblsl.git
cd liblsl && cmake -B build && cmake --build build --target install
```

---

## Verify installation

```bash
# Vulkan
vulkaninfo --summary

# OpenXR
openxr_example  # from openxr-sdk
```

---

## Build options

| Option | Default | Description |
|---|---|---|
| `SYNAPSE_BUILD_TESTS` | ON | Build test suite |
| `SYNAPSE_BUILD_EDITOR` | ON | Build ImGui editor tools |
| `SYNAPSE_USE_TRACY` | OFF | Tracy frame profiling |
| `SYNAPSE_WHISPER_VULKAN` | OFF | GPU-accelerated whisper.cpp |
| `SYNAPSE_BRAINFLOW_BOARDS` | OPENBCI;GANGLION;CYTON;EMOTIBIT | Which BCI boards to support |

## Build

```bash
cmake -B build -G Ninja \
    -DCMAKE_CXX_COMPILER=g++-13 \
    -DSYNAPSE_BUILD_TESTS=ON \
    -DSYNAPSE_BUILD_EDITOR=ON

cmake --build build -j$(nproc)
```

First build downloads and compiles all third-party libraries via FetchContent.
**Estimated time: 10–25 minutes** (Jolt Physics + whisper.cpp + GGML are the heaviest).

> 💡 First build is slow. Subsequent builds recompile only changed source files.
