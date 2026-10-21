#pragma once
#include <cstdint>

#include "numbers_converter.hpp"

namespace NodeSystem::strings::convert
{
    struct Ipv4Converter
    {
        using target_t = std::uint32_t;

        static std::optional<target_t> fromString(std::string_view ipStr)
        {
            uint32_t result = 0;
            int shift = 24;
            size_t start = 0;

            for (int i = 0; i < 4; ++i)
            {
                size_t end = (i == 3) ? ipStr.size() : ipStr.find('.', start);
                if (start >= ipStr.size() || (i < 3 && end == std::string_view::npos))
                {
                    return std::nullopt; // Порушено формат
                }

                const auto octetStr = ipStr.substr(start, end - start);
                auto octet = NumberConverter<std::uint8_t>::fromString(octetStr);
                if (!octet) return std::nullopt;

                result |= (static_cast<uint32_t>(*octet) << shift);
                shift -= 8;
                start = end + 1;
            }
            return result;
        }

        static std::string toString(const target_t& ip)
        {
            return std::format("{}.{}.{}.{}",
                (ip >> 24) & 0xFF,
                (ip >> 16) & 0xFF,
                (ip >> 8) & 0xFF,
                ip & 0xFF);
        }
    };

    static_assert(IsStringConverter<Ipv4Converter>);
}
