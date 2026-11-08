#pragma once
#include <nlohmann/json.hpp>

#include "diagnostic.hpp"
#include "../position.hpp"
#include "../../json_fields.hpp"
#include "../../methods.hpp"

namespace NodeSystem::lsp::protocol
{
    struct PublishDiagnosticsParams
    {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
        std::optional<std::int32_t> version{std::nullopt};

        [[nodiscard]]
        nlohmann::json to_json() const
        {
            nlohmann::json diag_array = nlohmann::json::array();
            for (const auto& item : diagnostics)
            {
                diag_array.push_back(item.to_json());
            }

            nlohmann::json j = {
                {json_names::uri, uri},
                {json_names::diagnostics, std::move(diag_array)}
            };

            if (version.has_value())
            {
                j[json_names::version] = *version;
            }

            return j;
        }

        [[nodiscard]]
        std::string dump_notification() const
        {
            const json_rpc::Notification notif{
                .method = std::string(methods::publish),
                .params = to_json()
            };

            std::string payload = json_rpc::JsonBuilder{}.toJson(notif).build().dump();
            return payload;
        }
    };
} // namespace NodeSystem::lsp::protocol
