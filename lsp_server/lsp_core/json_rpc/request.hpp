#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include "field_extractor.hpp"

namespace NodeSystem::lsp::json_rpc
{
    struct Request
    {
        nlohmann::json id;
        std::string method;
        std::optional<nlohmann::json> params{std::nullopt};

        static std::optional<Request> from_json(const nlohmann::json& j)
        {
            if (!JsonFieldExtractor::is_json_valid(j))
            {
                return std::nullopt;
            }
            const auto id = JsonFieldExtractor::extractId(j);
            if (!id)
            {
                return std::nullopt;
            }

            const auto method = JsonFieldExtractor::extractMethod(j);
            if (!method)
            {
                return std::nullopt;
            }

            auto params = JsonFieldExtractor::extractParams(j);

            return Request{
                .id = *id,
                .method = *method,
                .params = params
            };
        }
    };
} // namespace lsp::json_rpc
