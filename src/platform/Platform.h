#pragma once

#include <core/Types.h>

#include <string>

namespace synapse {

enum class OS : u8
{
    Unknown,
    Windows,
    Linux,
    macOS
};

struct PlatformInfo
{
    OS          os = OS::Unknown;
    std::string osName;
    std::string cpuName;
    u32         cpuCount = 0;
    u64         totalMemoryMB = 0;
    u64         availableMemoryMB = 0;
    std::string workingDirectory;
};

// ─────────────────────────────────────────────────────────────────────────
// Platform — HAL (todo 2.1). Toute dépendance OS passe par ici :
// info système, chemin de travail, sleep, horloge monotone, détection OS.
// Implémentation : src/platform/Platform.cpp (Linux /proc|sysconf, guard
// Windows).
// ─────────────────────────────────────────────────────────────────────────
class Platform
{
public:
    static void Init();
    static void Shutdown();

    static const PlatformInfo& GetInfo();
    static OS  CurrentOS() { return GetInfo().os; }
    static u32 CpuCount()  { return GetInfo().cpuCount; }

    static u64 AvailableMemoryMB();
    static std::string GetWorkingDirectory();
    static void Sleep(u32 milliseconds);
    static u64 GetTimeMs();   // monotonic, non-wall-clock

private:
    static PlatformInfo s_Info;
};

} // namespace synapse