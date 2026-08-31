#pragma once
#include <charconv>
#include <optional>
#include <string_view>
#include "nodes/NodeId.hpp"
#include "types/basic.hpp"

namespace NodeSystem::strings {

class ValueParser {
public:
    ValueParser() = delete;

    template <typename T>
    [[nodiscard]] static auto parse_number(std::string_view str) noexcept -> std::optional<T> {
        T value{};
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
        if (ec == std::errc{}) {
            return value;
        }
        return std::nullopt;
    }

    // Парсинг bool ("true", "false", "1", "0")
    [[nodiscard]] static auto parse_bool(std::string_view str) noexcept -> std::optional<bool> {
        if (str == "true" || str == "1") return true;
        if (str == "false" || str == "0") return false;
        return std::nullopt;
    }

    // Парсинг Direction ("up", "down")
    // Повертає 1 для "up" і 0 для "down" (або як у вас в Core)
    [[nodiscard]] static auto parse_direction(std::string_view str) noexcept -> std::optional<Core::types::Byte> {
        str = StringUtils::unquote(str);
        if (str == "up") return 1;
        if (str == "down") return 0;
        return std::nullopt;
    }

    // Парсинг KgNodeId (рівень.номер або просто номер)
    [[nodiscard]] static auto parse_node_id(std::string_view str) noexcept -> std::optional<Core::Nodes::NodeId> {
        Core::Nodes::NodeId id{};
        if (const auto dot = str.find('.'); dot != std::string_view::npos) {
            auto lvl = parse_number<Core::types::ID>(str.substr(0, dot));
            auto idx = parse_number<Core::types::ID>(str.substr(dot + 1));
            if (!lvl || !idx) return std::nullopt;
            id.level = *lvl;
            id.index = *idx;
        } else {
            auto idx = parse_number<Core::types::ID>(str);
            if (!idx) return std::nullopt;
            id.level = 0;
            id.index = *idx;
        }
        return id;
    }

    // Парсинг IPv4 (наприклад, "192.168.0.1")
    [[nodiscard]] static auto parse_ipv4(std::string_view str) noexcept -> std::optional<std::uint32_t> {
        auto octets = StringUtils::split_by_char(str, '.');
        if (octets.size() != 4) return std::nullopt;

        std::uint32_t ip = 0;
        for (int i = 0; i < 4; ++i) {
            auto octet = parse_number<std::uint32_t>(octets[i]);
            if (!octet || *octet > 255) return std::nullopt;
            ip |= (*octet << (24 - i * 8)); // Зсув залежно від endianness вашої архітектури
        }
        return ip;
    }
};

} // namespace NodeSystem::strings