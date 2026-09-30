#pragma once
#include "../../ParserContext.hpp"

namespace NodeSystem::strings::convert
{
    template <typename T>
    concept IsStructConverter =
        requires { typename T::target_t; } &&
        requires(std::string_view str, typename T::target_t val, const ParserContext& context)
        {
            { T::fromString(str, context) } -> std::same_as<std::optional<typename T::target_t>>;
            { T::toString(val) } -> std::same_as<std::string>;
            { T::toStringShort(val) } -> std::same_as<std::string>;
        };
    ;
}
