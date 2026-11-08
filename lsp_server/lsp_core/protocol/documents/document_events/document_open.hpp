#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../document_item.hpp"
#include "../../../json_fields.hpp"

namespace NodeSystem::lsp::protocol::documents
{
    struct DidOpenTextDocumentParams
    {
        DocumentItem textDocument;

        static std::optional<DidOpenTextDocumentParams> from_json(const nlohmann::json& j)
        {
            if (isFailedIntegrity(j))
            {
                return std::nullopt;
            }

            auto item = DocumentItem::from_json(j[json_names::textDocument]);
            if (!item.has_value())
            {
                return std::nullopt;
            }

            return DidOpenTextDocumentParams{.textDocument = std::move(*item)};
        }
    private:
        static bool isFailedIntegrity(const nlohmann::json& j)
        {
            return !j.is_object() || !j.contains( json_names::textDocument);
        }
    };
} // namespace NodeSystem::lsp::protocol
