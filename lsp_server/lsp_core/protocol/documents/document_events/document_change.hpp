#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../../../json_fields.hpp"

namespace NodeSystem::lsp::protocol::documents
{
    struct DidChangeTextDocumentParams
    {
        std::string uri;
        std::int32_t version{0};
        std::string text;

        static std::optional<DidChangeTextDocumentParams> from_json(const nlohmann::json& j)
        {
            if (isFailedIntegrity(j))
            {
                return std::nullopt;
            }

            const auto text = extractText(j);
            if (!text.has_value())
            {
                return std::nullopt;
            }

            auto [fst, snd] = extractDocInfo(j);

            return DidChangeTextDocumentParams{
                .uri = fst,
                .version = snd,
                .text = *text
            };
        }

    private:
        static bool isFailedIntegrity(const nlohmann::json& j)
        {
            return !j.is_object()
                || !j.contains(json_names::textDocument)
                || !j.contains(json_names::contentChanges);
        }

        static std::pair<decltype(uri), decltype(version)> extractDocInfo(const nlohmann::json& j)
        {
            const auto& doc_node = j[json_names::textDocument];
            return {
                doc_node.value(json_names::uri, ""),
                doc_node.value(json_names::version, 0)
            };
        }

        static std::optional<std::string> extractText(const nlohmann::json& j)
        {
            const auto& changes_node = j[json_names::contentChanges];
            if (!changes_node.is_array() || changes_node.empty())
            {
                return std::nullopt;
            }

            const auto& first_change = changes_node.front();
            if (!first_change.contains(json_names::text) ||
                !first_change[json_names::text].is_string())
            {
                return std::nullopt;
            }
            return first_change[json_names::text].get<std::string>();
        }
    };
} // namespace NodeSystem::lsp::protocol
