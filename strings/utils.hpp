#pragma once
#include <format>
#include <string>

#include "../coreі/types/ConfigTypes.hpp"
#include "types.hpp"
#include "algos/base_algo.hpp"


namespace NodeSystem::Core::strings {
    struct Utils {
        static constexpr auto ip_to_string(const types::IP host) {
            return std::to_string(host & 0xFF) + "." +
                   std::to_string((host >> 8) & 0xFF) + "." +
                   std::to_string((host >> 16) & 0xFF) + "." +
                   std::to_string((host >> 24) & 0xFF);
        }

        static constexpr auto ip_to_string(const types::KgIP ip) {
            return std::format("{}:{}", ip_to_string(ip.host), ip.port);
        }

        static constexpr auto ip_to_buffer(const types::IP ip) {
            std::array<char, 16> buffer;
            auto it = std::format_to(buffer.begin(), "{}.{}.{}.{}",
                                     (ip >> 24) & 0xFF,
                                     (ip >> 16) & 0xFF,
                                     (ip >> 8) & 0xFF,
                                     ip & 0xFF);
            *it = '\0';
            return buffer;
        }

        static constexpr auto ip_to_buffer(const types::KgIP ip) {
            std::array<char, 15 + 1 + 5 + 1> buffer;
            auto ip_buffer = ip_to_buffer(ip.host);
            auto it = std::format_to(buffer.begin(), "{}:{}", ip_buffer.data(), ip.port);
            *it = '\0';
            return buffer;
        }

        static auto ip_to_bufferConnection(const types::KgIP ip) {
            std::array<char, 64> buffer;
            auto it = std::format_to(buffer.begin(), "tcp://{}.{}.{}.{}:{}",
                                     (ip.host >> 24) & 0xFF,
                                     (ip.host >> 16) & 0xFF,
                                     (ip.host >> 8) & 0xFF,
                                     ip.host & 0xFF, ip.port);
            *it = '\0';
            return buffer;
        }

        static auto join(const std::span<const std::string_view> sequence, const std::string &separator) {
            if (sequence.empty()) return std::string{};

            size_t total = 0;
            for (const auto &s: sequence) total += s.size();
            total += separator.size() * (sequence.size() - 1);

            std::string result;
            result.reserve(total);

            for (size_t i = 0; i < sequence.size(); ++i) {
                if (i > 0) result += separator;
                result += sequence[i];
            }

            return result;
        }
    };
}
