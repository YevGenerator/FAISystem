#pragma once

#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "errors/error_codes.hpp"
#include "json_builder.hpp"
#include "field_extractor.hpp"
#include "notification.hpp"
#include "request.hpp"
#include "response.hpp"
#include "../expected.hpp"
#include "../json_fields.hpp"

namespace NodeSystem::lsp::json_rpc
{
    class Dispatcher
    {
    public:
        using RequestHandler = std::function<Response(const Request&)>;
        using NotificationHandler = std::function<void(const Notification&)>;

        void register_request(const std::string_view method, RequestHandler handler)
        {
            request_handlers_[std::string(method)] = std::move(handler);
        }

        void register_notification(const std::string_view method, NotificationHandler handler)
        {
            notification_handlers_[std::string(method)] = std::move(handler);
        }

        static std::string dump_response(const Response& response)
        {
            return JsonBuilder{}.toJson(response).build().dump();
        }

        static std::string dump_error(errors::ErrorCodeData& error,
                                      const std::optional<nlohmann::json>& id = std::nullopt)
        {
            const auto resp = Response::make_error(id, std::move(error));
            return dump_response(resp);
        }

        static Expected<nlohmann::json, std::string> parse_raw_json(std::string_view raw_json)
        {
            try
            {
                return nlohmann::json::parse(raw_json);
            }
            catch (const nlohmann::json::parse_error& e)
            {
                errors::ParseError err;
                err.buildExplanation("JSON syntax error: {}", e.what());
                return dump_error(err);
            }
        }

        static std::optional<std::string> verify_json(const nlohmann::json& j)
        {
            if (!j.is_object() || j.value(json_names::header, "") != json_names::header_value)
            {
                errors::InvalidRequestError err;
                err.buildExplanation("Missing or invalid 'jsonrpc' field");
                return dump_error(err);
            }
            return std::nullopt;
        }

        std::optional<std::string> tryParseRequest(const nlohmann::json& j) const
        {
            if (auto req = Request::from_json(j))
            {
                const auto it = request_handlers_.find(req->method);
                if (it == request_handlers_.end())
                {
                    errors::MethodNotFoundError err;
                    err.buildExplanation("Method '{}' is not registered", req->method);
                    return dump_error(err, req->id);
                }

                try
                {
                    Response resp = it->second(*req);
                    return dump_response(resp);
                }
                catch (const std::exception& e)
                {
                    errors::InternalError err;
                    err.buildExplanation("Exception during handler execution: {}", e.what());
                    return dump_error(err, req->id);
                }
            }
            return std::nullopt;
        }

        bool tryParseNotification(const nlohmann::json& j) const
        {
            if (const auto notif = Notification::from_json(j))
            {
                if (const auto it = notification_handlers_.find(notif->method);
                    it != notification_handlers_.end())
                {
                    try
                    {
                        it->second(*notif);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "[LSP Error] Notification '" << notif->method
                            << "' execution failed: " << e.what() << '\n';
                    }
                }
                return true;
            }
            return false;
        }

        static std::optional<std::string> tryParseMissingError(const nlohmann::json& j)
        {
            if (j.contains(json_names::id))
            {
                const auto id = JsonFieldExtractor::extractId(j);
                errors::InvalidRequestError err;
                err.buildExplanation("Invalid request structure or missing 'method'");
                return dump_error(err, id);
            }
            return std::nullopt;
        }

        [[nodiscard]] std::optional<std::string> dispatch(std::string_view raw_json) const
        {
            nlohmann::json parsed;
            if (auto result = parse_raw_json(raw_json); result.has_value())
            {
                parsed = *result;
            }
            else
            {
                return result.error();
            }
            if (auto error = verify_json(parsed); error.has_value())
            {
                return *error;
            }

            if (auto request = tryParseRequest(parsed); request.has_value())
            {
                return *request;
            }

            if (tryParseNotification(parsed))
            {
                return std::nullopt;
            }

            if (auto error = tryParseMissingError(parsed); error.has_value())
            {
                return *error;
            }

            return std::nullopt;
        }

    private:
        std::unordered_map<std::string, RequestHandler> request_handlers_;
        std::unordered_map<std::string, NotificationHandler> notification_handlers_;
    };
} // namespace NodeSystem::lsp::json_rpc
