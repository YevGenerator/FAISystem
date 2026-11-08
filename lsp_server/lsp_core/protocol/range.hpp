#pragma once
#include <nlohmann/json.hpp>

#include "position.hpp"
#include "../json_fields.hpp"

namespace NodeSystem::lsp::protocol
{
    struct Range
    {
        Position start;
        Position end;

        [[nodiscard]] nlohmann::json to_json() const
        {
            return {
                {json_names::start, start.to_json()},
                {json_names::end, end.to_json()}
            };
        }
    };
} // namespace NodeSystem::lsp::protocol
