#pragma once

#include <charconv>
#include <format>
#include <optional>
#include <string>

#include "../../lsp_core/rules/cursor.hpp"
#include "../../lsp_core/rules/rule_result.hpp"
#include "../../lsp_core/rules/syntax_node.hpp"
#include "../../lsp_core/rules/combinators/predicate_rule.hpp"
#include "../syntax/raw_value_token.hpp"

namespace NodeSystem::lsp::fai::types
{
    class Uint8Rule
    {
    public:
        [[nodiscard]] rules::RuleResult operator()(rules::SourceCursor& cursor) const
        {
            return rule_(cursor);
        }

    private:
        rules::combinators::PredicateRule<
            syntax::RawValueToken,
            std::optional<std::string>(*)(const rules::SyntaxNode&)
        > rule_{
            syntax::RawValueToken{},
            [](const rules::SyntaxNode& node) -> std::optional<std::string> {
                std::int64_t val = 0;
                auto [ptr, ec] = std::from_chars(node.text.data(), node.text.data() + node.text.size(), val);

                if (ec != std::errc() || ptr != (node.text.data() + node.text.size()))
                {
                    return std::format("Синтаксична помилка: значення '{}' не є валідним цілим числом", node.text);
                }

                if (val < 0 || val > 255)
                {
                    return std::format("Значення '{}' виходить за межі діапазону uint8 [0..255]", node.text);
                }

                return std::nullopt;
            }
        };
    };
} // namespace NodeSystem::lsp::fai::types