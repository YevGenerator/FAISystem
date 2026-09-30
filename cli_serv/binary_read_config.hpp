#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include "commands/CommandList.hpp"

namespace NodeSystem::Serv
{
    class BinaryConfigReader
    {
    public:
        template <typename TProcessor>
        static bool loadAndExecute(const std::filesystem::path& path, TProcessor& processor)
        {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.is_open())
            {
                std::cerr << "[ConfigReader] Error: Cannot open file: " << path << "\n";
                return false;
            }

            const auto size = file.tellg();
            if (size <= 0)
            {
                std::cerr << "[ConfigReader] Error: Config file is empty: " << path << "\n";
                return false;
            }

            file.seekg(0, std::ios::beg);
            std::vector<char> buffer(size);
            if (!file.read(buffer.data(), size))
            {
                std::cerr << "[ConfigReader] Error: Failed to read bytes: " << path << "\n";
                return false;
            }

            const char* ptr = buffer.data();
            std::size_t available = buffer.size();

            while (auto cmdOpt = Core::Commands::CommandList::nextFromFile(ptr, available))
            {
                std::visit([&processor]<typename TCommand>(const TCommand& concreteCmd)
                {
                    if constexpr (!std::is_same_v<std::decay_t<TCommand>, std::monostate>)
                    {
                        auto packet = Core::Commands::CommandList::CreatePacket(concreteCmd);
                        processor.process(packet);
                    }
                }, *cmdOpt);
            }

            return true;
        }
    };
} // namespace NodeSystem::IO
