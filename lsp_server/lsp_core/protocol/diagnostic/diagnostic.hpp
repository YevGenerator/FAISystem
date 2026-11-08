#pragma once
#include <nlohmann/json.hpp>

#include "../position.hpp"
#include "../range.hpp"
#include "../../json_fields.hpp"

namespace NodeSystem::lsp::protocol
{
    struct Diagnostic
    {
        using enum_t = std::uint8_t;

        enum class Severity : enum_t
        {
            Error = 1,
            Warning = 2,
            Information = 3,
            Hint = 4
        };

        Range range;
        Severity severity{Severity::Error};
        std::string message;
        std::optional<std::string> source{std::nullopt};
        std::optional<std::string> code{std::nullopt};

        [[nodiscard]]
        nlohmann::json to_json() const
        {
            nlohmann::json j = {
                {json_names::range, range.to_json()},
                {json_names::severity, static_cast<enum_t>(severity)},
                {json_names::message, message}
            };

            if (source.has_value())
            {
                j[json_names::source] = *source;
            }
            if (code.has_value())
            {
                j[json_names::code] = *code;
            }

            return j;
        }
    };
} // namespace NodeSystem::lsp::protocol
