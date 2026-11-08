#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "../../json_fields.hpp"

namespace NodeSystem::lsp::protocol::documents
{
    struct DocumentItem
    {
        std::string uri;
        std::string languageId;
        std::int32_t version{0};
        std::string text;

        static std::optional<DocumentItem> from_json(const nlohmann::json& j)
        {
            if (isFailedIntegrity(j))
            {
                return std::nullopt;
            }

            return DocumentItem{
                .uri = j.value(json_names::uri, ""),
                .languageId = j.value(json_names::language, ""),
                .version = j.value(json_names::version, 0),
                .text = j.value(json_names::text, "")
            };
        }

    private:
        static bool isFailedIntegrity(const nlohmann::json& j)
        {
            return !j.is_object()
                || !j.contains(json_names::uri)
                || !j.contains(json_names::text);
        }
    };
} // namespace NodeSystem::lsp::protocol
