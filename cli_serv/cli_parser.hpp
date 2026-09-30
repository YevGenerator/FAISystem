#pragma once
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace NodeSystem::Serv
{
    struct HostOptions
    {
        std::filesystem::path configPath = "output.bin";
        std::optional<std::uint32_t> deviceIdOverride = std::nullopt;
    };

    class CliParser
    {
    public:
        static HostOptions parse(int argc, char* argv[], bool isServer)
        {
            HostOptions options;

            if (argc < 2)
            {
                std::cout << (isServer ? "[Server]" : "[Servant]")
                    << " Enter config path [default: output.bin]: ";
                std::string input;
                std::getline(std::cin, input);
                if (!input.empty())
                {
                    if (input.front() == '"' && input.back() == '"')
                    {
                        input = input.substr(1, input.size() - 2);
                    }
                    options.configPath = input;
                }
                return options;
            }

            options.configPath = argv[1];

            for (int i = 2; i < argc; ++i)
            {
                std::string_view arg = argv[i];
                if ((arg == "-d" || arg == "--device") && (i + 1 < argc))
                {
                    options.deviceIdOverride = static_cast<std::uint32_t>(std::stoul(argv[++i]));
                }
            }

            return options;
        }
    };
} // namespace NodeSystem::CLI
