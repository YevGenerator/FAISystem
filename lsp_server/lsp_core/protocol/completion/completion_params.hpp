#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "completion_item_kind.hpp"
#include "../position.hpp"
#include "../../json_fields.hpp"

namespace NodeSystem::lsp::protocol
{
    struct CompletionParams
    {
        std::string uri;
        Position position;

        static std::optional<CompletionParams> from_json(const nlohmann::json& j)
        {
            if (isFailedIntegrity(j))
            {
                return std::nullopt;
            }

            const auto uri = extractUri(j);
            const auto pos = extractPosition(j);
            
            if (!uri.has_value() || !pos.has_value())
            {
                return std::nullopt;
            }

            return CompletionParams{
                .uri = *uri,
                .position = *pos
            };
        }

    private:
        static bool isFailedIntegrity(const nlohmann::json& j)
        {
            return !j.is_object()
                || !j.contains(json_names::textDocument)
                || !j.contains(json_names::position);
        }

        static std::optional<std::string> extractUri(const nlohmann::json& j)
        {
            const auto& doc_node = j[json_names::textDocument];
            if (!doc_node.is_object())
            {
                return std::nullopt;
            }
            return doc_node.value(json_names::uri, "");
        }

        static std::optional<Position> extractPosition(const nlohmann::json& j)
        {
            const auto& pos_node = j[json_names::position];

            if (!pos_node.is_object())
            {
                return std::nullopt;
            }
            return Position{
                .line = pos_node.value(json_names::line, 0u),
                .character = pos_node.value(json_names::character, 0u)
            };
        }
    };
} // namespace NodeSystem::lsp::protocol
