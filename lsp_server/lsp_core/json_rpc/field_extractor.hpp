#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <nlohmann/json.hpp>
#include "../json_fields.hpp"

namespace NodeSystem::lsp::json_rpc
{
    struct JsonFieldExtractor
    {
        static std::optional<nlohmann::json> extractId(const nlohmann::json& j)
        {
            if (!j.contains(json_names::id))
            {
                return std::nullopt;
            }
            const auto& id_node = j[json_names::id];
            if (id_node.is_number_integer() || id_node.is_string())
            {
                return std::optional{id_node};
            }
            return std::nullopt;
        }

        static std::optional<std::string> extractMethod(const nlohmann::json& j)
        {
            if (!j.contains(json_names::method) || !j[json_names::method].is_string())
            {
                return std::nullopt;
            }
            return j[json_names::method].get<std::string>();
        }

        static std::optional<nlohmann::json> extractParams(const nlohmann::json& j)
        {
            if (j.contains(json_names::params))
            {
                if (const auto& p = j[json_names::params];
                    p.is_object() || p.is_array())
                {
                    return std::optional{p};
                }
            }
            return std::nullopt;
        }

        constexpr static bool is_json_valid(const nlohmann::json& j)
        {
            return
                j.is_object() &&
                j.value(json_names::header, "") == json_names::header_value;
        }
    };
}
