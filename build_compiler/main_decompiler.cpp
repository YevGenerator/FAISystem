#include "cli_base.hpp"
#include "config_decompiler.hpp"

int main(int argc, char* argv[])
{
    return NodeSystem::Compiler::CliRunner::run(
        argc, argv,
        {
            .toolName = "Decompiler",
            .inputExt = ".bin",
            .defaultOutputExt = ".rpconfig"
        },
        [](const std::string& buffer, std::ostream& out) {
            return NodeSystem::Compiler::ConfigDecompiler::decompile(
                buffer.data(), buffer.size(), out
            );
        }
    );
}