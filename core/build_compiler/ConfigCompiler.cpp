#include <iostream>
#include <fstream>
#include <string>

#include "config_compiler.hpp"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cout << "Usage: rpcompiler <input.rpconfig> <output.bin>\n";
        std::string input, output;
        std::cout << "Input: ";
        std::cin >> input;
        std::cout << "\nOutput: ";
        std::cin >> output;
        NodeSystem::Compiler::ConfigCompiler::compile(input, output);
        std::cout << "\nDone!\n";
        return 1;
    }
    NodeSystem::Compiler::ConfigCompiler::compile(argv[1], argv[2]);
    return 0;
}
