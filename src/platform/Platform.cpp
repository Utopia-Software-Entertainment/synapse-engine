#include <platform/Platform.h>

#include <core/Logger.h>

#include <chrono>
#include <filesystem>
#include <thread>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <sys/sysinfo.h>
    #include <unistd.h>
#endif

namespace synapse {

PlatformInfo Platform::s_Info;

namespace {

OS DetectOS()
{
#if defined(_WIN32)
    return OS::Windows;
#elif defined(__APPLE__)
    return OS::macOS;
#elif defined(__linux__)
    return OS::Linux;
#else
    return OS::Unknown;
#endif
}

const char* OSName(OS os)
{
    switch (os)
    {
    case OS::Windows: return "Windows";
    case OS::Linux:   return "Linux";
    case OS::macOS:   return "macOS";
    default:          return "Unknown";
    }
}

u64 TotalMemoryMB()
{
#if defined(_WIN32)
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<u64>(status.ullTotalPhys) / (1024 * 1024);
#elif defined(__linux__)
    struct sysinfo info{};
    if (sysinfo(&info) == 0)
    {
        return (static_cast<u64>(info.totalram) * info.mem_unit) / (1024 * 1024);
    }
    return 0;
#else
    return 0;
#endif
}

u64 AvailableMemoryMB()
{
#if defined(_WIN32)
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<u64>(status.ullAvailPhys) / (1024 * 1024);
#elif defined(__linux__)
    struct sysinfo info{};
    if (sysinfo(&info) == 0)
    {
        return (static_cast<u64>(info.freeram) * info.mem_unit) / (1024 * 1024);
    }
    return 0;
#else
    return 0;
#endif
}

} // namespace

void Platform::Init()
{
    s_Info.os = DetectOS();
    s_Info.osName = OSName(s_Info.os);
    s_Info.cpuCount = std::max(1u, std::thread::hardware_concurrency());
    s_Info.totalMemoryMB = TotalMemoryMB();
    s_Info.availableMemoryMB = AvailableMemoryMB();
    s_Info.workingDirectory = std::filesystem::current_path().string();
    s_Info.cpuName = "unknown";

    SYNAPSE_CORE_INFO("Platform: {} | {} cores | {} MB RAM | cwd: {}",
                      s_Info.osName, s_Info.cpuCount, s_Info.totalMemoryMB,
                      s_Info.workingDirectory);
}

void Platform::Shutdown()
{
    SYNAPSE_CORE_INFO("Platform shutdown.");
}

const PlatformInfo& Platform::GetInfo()
{
    return s_Info;
}

u64 Platform::AvailableMemoryMB()
{
#if defined(__linux__)
    struct sysinfo info{};
    if (sysinfo(&info) == 0)
    {
        return (static_cast<u64>(info.freeram) * info.mem_unit) / (1024 * 1024);
    }
#endif
    return s_Info.availableMemoryMB;
}

std::string Platform::GetWorkingDirectory()
{
    return std::filesystem::current_path().string();
}

void Platform::Sleep(u32 milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

u64 Platform::GetTimeMs()
{
    using namespace std::chrono;
    return static_cast<u64>(duration_cast<milliseconds>(
                                steady_clock::now().time_since_epoch())
                                .count());
}

} // namespace synapse