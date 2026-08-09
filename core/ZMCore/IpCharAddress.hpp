#pragma once

#include <array>
#include <charconv>
#include <cstdint>
#include <string_view>

#include "commands/CmdTypes.hpp"

namespace NodeSystem::ZMCore {
    template<std::size_t BufferSize = 32>
    struct IpCharAddress {
        std::array<char, BufferSize> buf{};

        [[nodiscard]]
        auto c_str() const -> const char * { return buf.data(); }

        void makeAddress(const Core::Commands::KgIP &ip) {
            char *ptr = buf.data();
            char *end = buf.data() + BufferSize;

            constexpr std::string_view prefix = "tcp://";
            for (const char c: prefix) {
                *ptr++ = c;
            }

            const std::array bytes = {
                static_cast<uint8_t>((ip.host >> 24) & 0xFF),
                static_cast<uint8_t>((ip.host >> 16) & 0xFF),
                static_cast<uint8_t>((ip.host >> 8) & 0xFF),
                static_cast<uint8_t>(ip.host & 0xFF),
            };

            for (int i = 0; i < 4; ++i) {
                ptr = std::to_chars(ptr, end, bytes[i]).ptr;
                if (i < 3) {
                    *ptr++ = '.';
                }
            }

            *ptr++ = ':';
            ptr = std::to_chars(ptr, end, ip.port).ptr;
            *ptr = '\0';
        }

        void makeEmpty(const Core::Commands::KgIP &ip) {
            char *ptr = buf.data();
            char *end = buf.data() + BufferSize;

            constexpr std::string_view prefix = "tcp://*:";
            for (const char c: prefix) {
                *ptr++ = c;
            }

            ptr = std::to_chars(ptr, end, ip.port).ptr;
            *ptr = '\0';
        }

        static auto ipCharAddress(const Core::Commands::KgIP &ip) -> IpCharAddress {
            IpCharAddress addr;
            addr.makeAddress(ip);
            return addr;
        }

        static auto ipCharEmptyAddress(const Core::Commands::KgIP &ip) -> IpCharAddress {
            IpCharAddress addr;
            addr.makeEmpty(ip);
            return addr;
        }
    };
} // namespace NodeSystem::ZMCore
