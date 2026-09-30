#pragma once
#include "is_string_converter.hpp"
#include <charconv>
#include <format>

namespace NodeSystem::strings::convert
{
    struct NodeIdConverter
    {
        using target_t = dto::NodeId;

        static std::optional<target_t> fromString(std::string_view str)
        {
            size_t dotPos = str.find('.');
            if (dotPos == std::string_view::npos)
            {
                return std::nullopt;
            }

            Core::types::ID level = 0, index = 0;
            auto [p1, ec1] = std::from_chars(str.data(), str.data() + dotPos, level);
            auto [p2, ec2] = std::from_chars(str.data() + dotPos + 1, str.data() + str.size(), index);

            if (ec1 == std::errc() && ec2 == std::errc())
            {
                return target_t{level, index};
            }
            return std::nullopt;
        }

        static std::string toString(const target_t& val)
        {
            return std::format("{}.{}", val.level, val.index);
        }
    };

    static_assert(IsStringConverter<NodeIdConverter>);
}
