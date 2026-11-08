#pragma once

#include "../../lsp_core/rules/cursor.hpp"
#include "../../lsp_core/rules/rule_result.hpp"

namespace NodeSystem::lsp::fai::syntax
{
    class RawValueToken
    {
    public:
        [[nodiscard]] rules::RuleResult operator()(rules::SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();

            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value()) break;

                if (*ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n' ||
                    *ch == ';' || *ch == '<' || *ch == '>' || *ch == '[' || *ch == ']')
                {
                    break;
                }
                cursor.advance();
            }

            const auto token_text = cursor.slice_since(cp);
            if (token_text.empty())
            {
                cursor.restore(cp);
                return rules::RuleResult::mismatch();
            }

            return rules::RuleResult::success(rules::SyntaxNode{
                .tag = "raw_value",
                .text = token_text,
                .range = cursor.range_since(cp),
                .children = {}
            });
        }
    };
} // namespace NodeSystem::lsp::fai::syntax::tokens