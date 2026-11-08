#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../document_item.hpp"
#include "../../../json_fields.hpp"

namespace NodeSystem::lsp::protocol::documents
{
    struct DidCloseTextDocumentParams
    {
        std::string uri;

        static std::optional<DidCloseTextDocumentParams> from_json(const nlohmann::json& j)
        {
            if (isFailedIntegrity(j))
            {
                return std::nullopt;
            }

            return DidCloseTextDocumentParams{.uri = extractUri(j)};
        }

    private:
        static bool isFailedIntegrity(const nlohmann::json& j)
        {
            return !j.is_object() || !j.contains(json_names::textDocument);
        }

        static std::string extractUri(const nlohmann::json& j)
        {
            return j[json_names::textDocument].value(json_names::uri, "");
        }
    };
} // namespace NodeSystem::lsp::protocol
