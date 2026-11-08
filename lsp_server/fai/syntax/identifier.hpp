#pragma once

#include <cctype>

#include "../../lsp_core/rules/cursor.hpp"
#include "../../lsp_core/rules/rule_result.hpp"

namespace NodeSystem::lsp::fai::syntax
{
    class Identifier
    {
    public:
        [[nodiscard]] rules::RuleResult operator()(rules::SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();
            const auto first = cursor.peek();

            if (!first.has_value() || (!std::isalpha(static_cast<unsigned char>(*first)) && *first != '_'))
            {
                return rules::RuleResult::mismatch();
            }

            cursor.advance();
            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value() || (!std::isalnum(static_cast<unsigned char>(*ch)) && *ch != '_'))
                {
                    break;
                }
                cursor.advance();
            }

            return rules::RuleResult::success(rules::SyntaxNode{
                .tag = "identifier",
                .text = cursor.slice_since(cp),
                .range = cursor.range_since(cp),
                .children = {}
            });
        }
    };
} // namespace NodeSystem::lsp::fai::syntax::tokens