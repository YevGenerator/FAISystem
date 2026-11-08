#pragma once

#include <cstdint>
#include <optional>
#include <nlohmann/json.hpp>
#include "errors/error_codes.hpp"

namespace NodeSystem::lsp::json_rpc
{
    struct Response
    {
        std::optional<nlohmann::json> id{std::nullopt};
        std::optional<nlohmann::json> data;
        std::optional<errors::ErrorCodeData> error;

        static Response make_success(nlohmann::json requestId, nlohmann::json result)
        {
            return {
                .id = std::move(requestId),
                .data = std::move(result),
                .error = std::nullopt
            };
        }

        static Response make_error(const std::optional<nlohmann::json>& requestId,
                                   errors::ErrorCodeData code,
                                   std::optional<nlohmann::json> data = std::nullopt)
        {
            return {
                .id = requestId,
                .data = std::move(data),
                .error = std::move(code)
            };
        }
    };
} // namespace lsp::json_rpc
