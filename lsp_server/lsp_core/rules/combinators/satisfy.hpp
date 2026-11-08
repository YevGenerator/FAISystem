#pragma once

#include <concepts>
#include <utility>

#include "../cursor.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <typename Predicate>
    requires std::predicate<Predicate, char>
    class Satisfy
    {
    public:
        constexpr explicit Satisfy(Predicate pred) noexcept(std::is_nothrow_move_constructible_v<Predicate>)
            : predicate_(std::move(pred))
        {
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();
            const auto ch = cursor.peek();

            if (!ch.has_value() || !predicate_(*ch))
            {
                cursor.restore(cp);
                return RuleResult::mismatch();
            }

            cursor.advance();

            return RuleResult::success(SyntaxNode{
                .tag = "char",
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
    };
} // namespace NodeSystem::lsp::rules::combinators