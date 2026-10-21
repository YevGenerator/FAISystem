#pragma once
#include <charconv>
#include <format>

#include "is_string_converter.hpp"
#include "../../dto/dto.hpp"

namespace NodeSystem::strings::convert
{
    template <std::integral T = std::uint8_t>
    struct NumberConverter
    {
        using target_t = T;

        static std::optional<target_t> fromString(std::string_view str)
        {
            target_t value{};
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
            if (ec == std::errc() && ptr == str.data() + str.size())
            {
                return value;
            }
            return std::nullopt;
        }

        static auto toString(const T& val)
        {
            return std::to_string(val);
        }
    };

    using IdConverter = NumberConverter<dto::Id>;

    static_assert(IsStringConverter<NumberConverter<>>);
    static_assert(IsStringConverter<IdConverter>);


    struct DoubleConverter
    {
        using target_t = dto::Float;

        static std::optional<target_t> fromString(std::string_view str)
        {
            try
            {
                size_t idx = 0;
                std::string s(str);
                return std::stod(s, &idx);
            }
            catch (...)
            {
            }
            return std::nullopt;
        }

        static auto toString(const target_t& val)
        {
            return std::format("{:.4f}", val);
        }
    };

    static_assert(IsStringConverter<DoubleConverter>);
}
