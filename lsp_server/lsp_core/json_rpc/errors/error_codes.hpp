#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

#include "../../fixed_string.hpp"

namespace NodeSystem::lsp::json_rpc::errors
{
    struct ErrorCodeData
    {
        std::int32_t code;
        std::string_view message;
        std::optional<std::string> explanation;

        template <typename... Args>
        void buildExplanation(std::format_string<Args...> format, Args&&... args)
        {
            this->explanation = std::format(format, std::forward<Args>(args)...);
        }

        [[nodiscard]]
        std::string toString() const
        {
            if (explanation.has_value())
            {
                return  std::format("[{}] {}: {}", code, message, *explanation);
            }
            return std::format("[{}] {}", code, message);
        }
    };

    template <std::int32_t Code, FixedString Message>
    struct ErrorCode : ErrorCodeData
    {
        constexpr ErrorCode() : ErrorCodeData{Code, Message}
        {
        }
    };

    using ParseError = ErrorCode<-32700, "Parse Error">;
    using InvalidRequestError = ErrorCode<-32600, "Invalid Request">;
    using MethodNotFoundError = ErrorCode<-32601, "Method not found">;
    using InvalidParamsError = ErrorCode<-32602, "Invalid params error">;
    using InternalError = ErrorCode<-32603, "Internal error">;
    using ServerNotInitializedError = ErrorCode<-32002, "Server not initialized">;
}
