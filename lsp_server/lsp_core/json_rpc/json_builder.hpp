#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include <nlohmann/json.hpp>

#include "notification.hpp"
#include "response.hpp"
#include "errors/error_codes.hpp"

namespace NodeSystem::lsp::json_rpc
{
    struct JsonBuilder
    {
        nlohmann::json j;

        JsonBuilder& init_header()
        {
            j = {
                {json_names::header, json_names::header_value}
            };
            return *this;
        }

        JsonBuilder& addMethod(const std::string& method)
        {
            j[json_names::method] = method;
            return *this;
        }

        JsonBuilder& addParams(const std::optional<nlohmann::json>& params)
        {
            if (params.has_value())
            {
                j[json_names::params] = *params;
            }
            return *this;
        }

        JsonBuilder& addId(const std::optional<nlohmann::json>& id)
        {
            if (id.has_value())
            {
                j[json_names::id] = *id;
            }
            else
            {
                j[json_names::id] = nullptr;
            }
            return *this;
        }

        JsonBuilder& addError(const std::optional<errors::ErrorCodeData>& code)
        {
            if (code.has_value())
            {
                nlohmann::json err_node = {
                    {json_names::code, code.value().code},
                    {json_names::message, code.value().toString()}
                };
                j[json_names::error] = std::move(err_node);
            }


            return *this;
        }


        JsonBuilder& addResult(const Response& response)
        {
            if (response.error.has_value())
            {
                if (response.data.has_value())
                {
                    j[json_names::error][json_names::error_data] = *response.data;
                }
            }
            else
            {
                if (response.data.has_value())
                {
                    j[json_names::result] = *response.data;
                }
                else
                {
                    j[json_names::result] = nullptr;
                }
            }
            return *this;
        }

        JsonBuilder& toJson(const Response& response)
        {
            return init_header().addId(response.id).addError(response.error).addResult(response);
        }

        JsonBuilder& toJson(const Notification& notification)
        {
            return init_header().addMethod(notification.method).addParams(notification.params);
        }

        [[nodiscard]]
        nlohmann::json build() &&
        {
            return std::move(j);
        }

        [[nodiscard]] nlohmann::json build() const &
        {
            return j;
        }
    };
}
