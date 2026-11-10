#pragma once
#include <cstdint>
#include <charconv>
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
        std::optional<std::filesystem::path> logPath = std::nullopt;
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

            int firstFlagIndex = 2;
            if (!isServer && argc >= 3) {
                const std::string_view idText = argv[1];
                std::uint32_t legacyId{};
                const auto parsed = std::from_chars(idText.data(), idText.data() + idText.size(), legacyId);
                if (parsed.ec == std::errc{} && parsed.ptr == idText.data() + idText.size()) {
                    options.deviceIdOverride = legacyId;
                    options.configPath = argv[2];
                    firstFlagIndex = 3;
                } else {
                    options.configPath = argv[1];
                }
            } else {
                options.configPath = argv[1];
            }

            for (int i = firstFlagIndex; i < argc; ++i)
            {
                std::string_view arg = argv[i];
                if ((arg == "-d" || arg == "--device") && (i + 1 < argc))
                {
                    options.deviceIdOverride = static_cast<std::uint32_t>(std::stoul(argv[++i]));
                } else if (arg == "--log" && (i + 1 < argc)) {
                    options.logPath = std::filesystem::path(argv[++i]);
                }
            }

            return options;
        }
    };
} // namespace NodeSystem::CLI
