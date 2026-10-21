#pragma once
#include "is_string_converter.hpp"
#include "../../dto/dto.hpp"

namespace NodeSystem::strings::convert
{
    struct NetBoolConverter
    {
        using target_t = dto::Bool;

        static std::optional<target_t> fromString(std::string_view str)
        {
            if (str == "up" || str == "\"up\"" || str == "true" || str == "1")
            {
                return true;
            }
            if (str == "down" || str == "\"down\"" || str == "false" || str == "0")
            {
                return false;
            }
            return std::nullopt;
        }

        static std::string toString(const target_t& val)
        {
            return val.get() ? "\"up\"" : "\"down\"";
        }
    };

    static_assert(IsStringConverter<NetBoolConverter>);
}
