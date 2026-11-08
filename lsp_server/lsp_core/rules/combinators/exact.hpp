#pragma once

#include <string_view>

#include "../cursor.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    class Exact
    {
    public:
        constexpr explicit Exact(std::string_view expected) noexcept
            : expected_(expected)
        {
        }

        [[nodiscard]]
        constexpr std::string_view expected() const noexcept
        {
            return expected_;
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const noexcept
        {
            const auto cp = cursor.mark();

            if (!cursor.consume_prefix(expected_))
            {
                cursor.restore(cp);
                return RuleResult::mismatch();
            }

            return RuleResult::success(SyntaxNode{
                .tag = "exact",
                .text = cursor.slice_since(cp),
                .range = cursor.range_since(cp),
                .children = {}
            });
        }

        RuleResult operator()(SourceCursor& cursor) const noexcept
        {
            return match(cursor);
        }

    private:
        std::string_view expected_;
    };
} // namespace NodeSystem::lsp::rules::combinators