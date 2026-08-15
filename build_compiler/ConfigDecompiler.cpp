#include "config_decompiler.hpp"


int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: rpdecompiler <input.bin> <output.rpconfig>\n";
        std::string input, output;
        std::cout << "Input: ";
        std::cin >> input;
        std::cout << "\nOutput: ";
        std::cin >> output;
        NodeSystem::Compiler::ConfigDecompiler::decompile(input, output);
        std::cout << "Decompilation completed successfully.\n";
        std::cout << "\nDone!\n";
        return 1;
    }
    NodeSystem::Compiler::ConfigDecompiler::decompile(argv[1], argv[2]);
    return 0;
}
