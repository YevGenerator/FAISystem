#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <filesystem>

namespace NodeSystem::Compiler
{
    namespace fs = std::filesystem;

    struct ToolConfig
    {
        std::string_view toolName;
        std::string_view inputExt;
        std::string_view defaultOutputExt;
    };

    class CliRunner
    {
    private:
        static void sanitizePath(std::string& path)
        {
            auto trimWhitespace = [](std::string& s) {
                auto start = s.find_first_not_of(" \t\n\r");
                if (start == std::string::npos) {
                    s.clear();
                    return;
                }
                auto end = s.find_last_not_of(" \t\n\r");
                s = s.substr(start, end - start + 1);
            };

            trimWhitespace(path);
            if (path.size() >= 2 && ((path.front() == '"' && path.back() == '"') ||
                                     (path.front() == '\'' && path.back() == '\'')))
            {
                path = path.substr(1, path.size() - 2);
                trimWhitespace(path);
            }
        }

        static bool resolvePaths(int argc, char* argv[], const ToolConfig& config,
                                 std::string& inputPath, std::string& outputPath)
        {
            if (argc >= 2)
            {
                inputPath = argv[1];
                if (argc >= 3)
                {
                    outputPath = argv[2];
                }
            }
            else
            {
                std::cout << "Enter input file path (" << config.inputExt << "): ";
                std::getline(std::cin, inputPath);
                sanitizePath(inputPath);

                if (inputPath.empty())
                {
                    std::cerr << "[Error] Input path cannot be empty.\n";
                    return false;
                }

                std::string defaultOut = fs::path(inputPath).replace_extension(config.defaultOutputExt).string();

                std::cout << "Enter output file path [Press Enter for default: " << defaultOut << "]: ";
                std::getline(std::cin, outputPath);
                sanitizePath(outputPath);

                if (outputPath.empty())
                {
                    outputPath = defaultOut;
                }
            }

            sanitizePath(inputPath);
            sanitizePath(outputPath);

            if (outputPath.empty())
            {
                outputPath = fs::path(inputPath).replace_extension(config.defaultOutputExt).string();
            }

            return true;
        }

    public:
        template <typename Action>
        static int run(int argc, char* argv[], const ToolConfig& config, Action&& action)
        {
            std::string inputPath;
            std::string outputPath;

            if (!resolvePaths(argc, argv, config, inputPath, outputPath))
            {
                return 1;
            }

            std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
            if (!inFile.is_open())
            {
                std::cerr << "[Error] Failed to open input file: " << inputPath << "\n";
                return 1;
            }

            const auto fileSize = inFile.tellg();
            inFile.seekg(0, std::ios::beg);

            std::string buffer(fileSize, '\0');
            if (fileSize > 0 && !inFile.read(buffer.data(), fileSize))
            {
                std::cerr << "[Error] Failed to read content from: " << inputPath << "\n";
                return 1;
            }
            inFile.close();

            std::ofstream outFile(outputPath, std::ios::binary | std::ios::trunc);
            if (!outFile.is_open())
            {
                std::cerr << "[Error] Failed to open output file for writing: " << outputPath << "\n";
                return 1;
            }

            if (!action(buffer, outFile))
            {
                std::cerr << "[Error] " << config.toolName << " failed due to syntax or data errors.\n";
                return 1;
            }

            std::cout << "[OK] " << config.toolName << " succeeded: " << inputPath << " -> " << outputPath << "\n";
            return 0;
        }
    };
}