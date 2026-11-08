#pragma once

#include <string>
#include <utility>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"
#include "../../protocol/diagnostic/diagnostic.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule InnerRule>
    class Expect
    {
    public:
        Expect(InnerRule rule, std::string error_message)
            : rule_(std::move(rule)), message_(std::move(error_message))
        {
        }

        [[nodiscard]] RuleResult match(SourceCursor& cursor) const
        {
            const auto start_cp = cursor.mark();
            auto res = rule_(cursor);

            if (res.is_mismatch())
            {
                cursor.restore(start_cp);
                const auto err_start_cp = cursor.mark();

                if (!cursor.is_eof() && cursor.peek() != '\n' && cursor.peek() != '\r')
                {
                    while (!cursor.is_eof())
                    {
                        const auto ch = cursor.peek();
                        if (!ch.has_value() || *ch == ' ' || *ch == '\t' || *ch == '\r' || *ch == '\n' ||
                            *ch == '=' || *ch == '<' || *ch == '>' || *ch == '[' || *ch == ']')
                        {
                            break;
                        }
                        cursor.advance();
                    }
                }

                auto err_range = cursor.range_since(err_start_cp);
                
                if (err_range.start.line == err_range.end.line &&
                    err_range.start.character == err_range.end.character)
                {
                    err_range.end.character = err_range.start.character + 1;
                }

                return RuleResult::failure(protocol::Diagnostic{
                    .range = err_range,
                    .severity = protocol::Diagnostic::Severity::Error,
                    .message = message_
                });
            }

            return res;
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        InnerRule rule_;
        std::string message_;
    };

    template <Rule InnerRule>
    Expect(InnerRule, std::string) -> Expect<InnerRule>;
} // namespace NodeSystem::lsp::rules::combinators