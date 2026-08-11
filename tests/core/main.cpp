#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <core/Logger.h>

#include <filesystem>

int main(int argc, char** argv)
{
    std::filesystem::create_directories("logs");
    synapse::Logger::Init();

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    const int result = context.run();
    return result;
}
