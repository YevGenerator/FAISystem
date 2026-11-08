#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <nlohmann/json.hpp>

#include "../json_fields.hpp"

namespace NodeSystem::lsp::protocol
{
    struct Position
    {
        std::uint32_t line{0};
        std::uint32_t character{0};

        [[nodiscard]]
        nlohmann::json to_json() const
        {
            return {
                {json_names::line, line},
                {json_names::character, character}
            };
        }
    };


} // namespace NodeSystem::lsp::protocol
