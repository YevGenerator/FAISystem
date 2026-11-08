#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "../cursor.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <typename Predicate>
    requires std::predicate<Predicate, char>
    class TakeWhile
    {
    public:
        constexpr explicit TakeWhile(Predicate pred, std::size_t min_count = 1) noexcept(std::is_nothrow_move_constructible_v<Predicate>)
            : predicate_(std::move(pred)), min_count_(min_count)
        {
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();
            std::size_t matched_count = 0;

            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value() || !predicate_(*ch))
                {
                    break;
                }
                cursor.advance();
                ++matched_count;
            }

            if (matched_count < min_count_)
            {
                cursor.restore(cp);
                return RuleResult::mismatch();
            }

            return RuleResult::success(SyntaxNode{
                .tag = "token",
                .text = cursor.slice_since(cp),
                .range = cursor.range_since(cp),
                .children = {}
            });
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        Predicate predicate_;
        std::size_t min_count_{1};
    };
} // namespace NodeSystem::lsp::rules::combinators