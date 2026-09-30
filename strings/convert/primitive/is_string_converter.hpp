#pragma once
#include <optional>
#include <string_view>

namespace NodeSystem::strings::convert
{
    template <typename T>
    concept IsStringConverter =
        requires { typename T::target_t; } &&
        requires(std::string_view str, typename T::target_t val)
        {
            { T::fromString(str) } -> std::same_as<std::optional<typename T::target_t>>;
            { T::toString(val) } -> std::same_as<std::string>;
        };
    ;
}
