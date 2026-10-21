#pragma once

#include "../util_quotes.hpp"
#include "../../algoname_map.hpp"
#include "../../dto/dto.hpp"

namespace NodeSystem::strings::convert
{
    struct AlgoNameConverter
    {
        using target_t = dto::AlgoType;

        static constexpr auto fallback = "_";

        static std::optional<target_t> fromString(std::string_view str)
        {
            return AlgoNames::algoId(UtilQuotes::from_quotes(str));
        }

        static std::string toString(const target_t& val)
        {
            auto name = AlgoNames::algoName(val);
            if (name.has_value())
            {
                return UtilQuotes::in_quotes(*name);
            }
            return UtilQuotes::in_quotes(fallback);
        }
    };

    static_assert(IsStringConverter<AlgoNameConverter>);
}
