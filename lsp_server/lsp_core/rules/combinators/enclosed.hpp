#pragma once

#include <string>
#include <utility>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"
#include "trivia.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule OpenRule, Rule ContentRule, Rule CloseRule>
    class Enclosed
    {
    public:
        Enclosed(OpenRule open_rule,
                 ContentRule content_rule,
                 CloseRule close_rule,
                 std::string unclosed_error_message = "Синтаксична помилка: очікувався закриваючий символ")
            : open_rule_(std::move(open_rule)),
              content_rule_(std::move(content_rule)),
              close_rule_(std::move(close_rule)),
              error_message_(std::move(unclosed_error_message))
        {
        }

        [[nodiscard]] RuleResult match(SourceCursor& cursor) const
        {
            const auto start_cp = cursor.mark();

            auto open_res = open_rule_(cursor);
            if (!open_res.is_success())
            {
                cursor.restore(start_cp);
                return open_res; // mismatch або failure
            }

            Trivia::skip_all(cursor);

            auto content_res = content_rule_(cursor);
            if (content_res.is_failure())
            {
                return content_res;
            }

            Trivia::skip_all(cursor);
            
            auto close_res = close_rule_(cursor);

            if (!close_res.is_success())
            {
                return RuleResult::failure(protocol::Diagnostic{
                    .range = cursor.range_since(start_cp),
                    .severity = protocol::Diagnostic::Severity::Error,
                    .message = error_message_
                });
            }

            SyntaxNode block_node{
                .tag = "enclosed",
                .text = cursor.slice_since(start_cp),
                .range = cursor.range_since(start_cp),
                .children = {}
            };

            if (content_res.is_success())
            {
                block_node.children.push_back(std::move(content_res.node()));
            }

            return RuleResult::success(std::move(block_node));
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        OpenRule open_rule_;
        ContentRule content_rule_;
        CloseRule close_rule_;
        std::string error_message_;
    };
} // namespace NodeSystem::lsp::rules::combinators