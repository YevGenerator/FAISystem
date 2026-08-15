#pragma once

#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "../coreі/commands/CommandList.hpp"

namespace NodeSystem::Core {
    constexpr std::uint32_t MAGIC_BYTES = 0x46435052; // "RPCF"
    constexpr std::uint16_t FORMAT_VERSION = 1;

#pragma pack(push, 1)
    struct FileHeader {
        std::uint32_t magic;
        std::uint16_t version;
    };

    constexpr auto fileHeaderSize = sizeof(FileHeader);

#pragma pack(pop)

    class ConfigFileReader {
        std::vector<char> buffer;
        const char *ptr = nullptr;
        std::size_t available = 0;

    public:
        bool load(const std::string &path) {
            std::ifstream f(path, std::ios::binary | std::ios::ate);
            if (!f) return false;
            const auto size = f.tellg();
            if (size < sizeof(FileHeader)) return false;
            f.seekg(0);
            buffer.resize(size);
            f.read(buffer.data(), size);

            const auto *hdr = reinterpret_cast<FileHeader *>(buffer.data());
            if (hdr->magic != MAGIC_BYTES) return false;
            if (hdr->version != FORMAT_VERSION) {
                std::cerr << "Version mismatch! Expected " << FORMAT_VERSION << ", got " << hdr->version << "\n";
                return false;
            }

            ptr = buffer.data() + fileHeaderSize;
            available = static_cast<std::size_t>(size) - fileHeaderSize;
            return true;
        }

        std::optional<CommandList::Variant> next() {
            return CommandList::nextFromFile(ptr, available);
        }
    };
}
