#include "cli_base.hpp"
#include "config_compiler.hpp"

int main(int argc, char* argv[])
{
    return NodeSystem::Compiler::CliRunner::run(
        argc, argv,
        {
            .toolName = "Compiler",
            .inputExt = ".rpconfig",
            .defaultOutputExt = ".bin"
        },
        [](const std::string& source, std::ostream& out) {
            return NodeSystem::Compiler::ConfigCompiler::compile(source, out);
        }
    );
}