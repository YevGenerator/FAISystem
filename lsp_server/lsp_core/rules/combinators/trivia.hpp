#pragma once

#include <string_view>

#include "../cursor.hpp"
#include "../rule_result.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    class Trivia
    {
    public:
        [[nodiscard]]
        static constexpr bool is_horizontal_space(const char ch) noexcept
        {
            return ch == ' ' || ch == '\t';
        }

        [[nodiscard]]
        static constexpr bool is_newline(const char ch) noexcept
        {
            return ch == '\n' || ch == '\r';
        }

        static constexpr void skip_horizontal(SourceCursor& cursor) noexcept
        {
            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value() || !is_horizontal_space(*ch))
                {
                    break;
                }
                cursor.advance();
            }
        }

        static constexpr void skip_newlines(SourceCursor& cursor) noexcept
        {
            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value() || !is_newline(*ch))
                {
                    break;
                }
                cursor.advance();
            }
        }

        static constexpr void skip_comment(SourceCursor& cursor) noexcept
        {
            if (cursor.starts_with("#") || cursor.starts_with("//"))
            {
                while (!cursor.is_eof())
                {
                    const auto ch = cursor.peek();
                    if (!ch.has_value() || *ch == '\n' || *ch == '\r')
                    {
                        break;
                    }
                    cursor.advance();
                }
            }
        }

        static constexpr void skip_all(SourceCursor& cursor) noexcept
        {
            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value())
                {
                    break;
                }

                if (is_horizontal_space(*ch) || is_newline(*ch))
                {
                    cursor.advance();
                }
                else if (cursor.starts_with("#") || cursor.starts_with("//"))
                {
                    skip_comment(cursor);
                }
                else
                {
                    break;
                }
            }
        }
        
        RuleResult operator()(SourceCursor& cursor) const noexcept
        {
            const auto cp = cursor.mark();
            skip_all(cursor);

            return RuleResult::success(SyntaxNode{
                .tag = "trivia",
                .text = cursor.slice_since(cp),
                .range = cursor.range_since(cp),
                .children = {}
            });
        }
    };
} // namespace NodeSystem::lsp::rules::combinators
