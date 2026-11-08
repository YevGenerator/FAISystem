#pragma once
#include <algorithm>
#include <cstddef>
#include <string>
#include <variant>

namespace NodeSystem::lsp
{
    template <typename TExpected, typename TError>
    struct Expected
    {
        std::variant<TExpected, TError> data;

        constexpr bool has_value()
        {
            return std::holds_alternative<TExpected>(data);
        }

        constexpr TExpected& value()
        {
            return std::get<TExpected>(data);
        }

        constexpr TError& error()
        {
            return std::get<TError>(data);
        }

        constexpr TExpected& operator*()
        {
            return value();
        }

        constexpr Expected(TExpected success) : data(std::move(success))
        {
        }

        constexpr Expected(TError error) : data(std::move(error))
        {
        }
    };
}
