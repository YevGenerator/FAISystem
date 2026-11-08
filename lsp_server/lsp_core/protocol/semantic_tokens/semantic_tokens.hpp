#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "../position.hpp"

namespace NodeSystem::lsp::protocol
{
    enum class SemanticTokenType : std::uint32_t
    {
        Keyword = 0,
        Type = 1,
        Variable = 2,
        Number = 3,
        Operator = 4
    };

    struct SemanticTokensLegend
    {
        static const std::vector<std::string>& types()
        {
            static const std::vector<std::string> token_types = {
                "keyword",
                "type",
                "variable",
                "number",
                "operator"
            };
            return token_types;
        }

        [[nodiscard]] static nlohmann::json to_json()
        {
            return {
                {"tokenTypes", types()},
                {"tokenModifiers", nlohmann::json::array()}
            };
        }
    };

    class SemanticTokensBuilder
    {
    public:
        void add_token(Position pos, std::uint32_t length, SemanticTokenType type)
        {
            std::uint32_t delta_line = pos.line - prev_line_;
            std::uint32_t delta_char = (delta_line == 0) ? (pos.character - prev_char_) : pos.character;

            data_.push_back(delta_line);
            data_.push_back(delta_char);
            data_.push_back(length);
            data_.push_back(static_cast<std::uint32_t>(type));
            data_.push_back(0); // modifier bitmask

            prev_line_ = pos.line;
            prev_char_ = pos.character;
        }

        [[nodiscard]] const std::vector<std::uint32_t>& data() const noexcept
        {
            return data_;
        }

        [[nodiscard]] nlohmann::json to_json() const
        {
            return {
                {"data", data_}
            };
        }

    private:
        std::vector<std::uint32_t> data_;
        std::uint32_t prev_line_{0};
        std::uint32_t prev_char_{0};
    };
} // namespace NodeSystem::lsp::protocol