#pragma once

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"
#include "trivia.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule... Rules>
    requires (sizeof...(Rules) > 0)
    class Sequence
    {
    public:
        explicit Sequence(Rules... rules)
            : rules_(std::move(rules)...)
        {
        }

        Sequence(bool auto_skip_trivia, Rules... rules)
            : rules_(std::move(rules)...), skip_trivia_(auto_skip_trivia)
        {
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();

            SyntaxNode parent_node{
                .tag = "sequence",
                .text = {},
                .range = {},
                .children = {}
            };
            parent_node.children.reserve(sizeof...(Rules));

            bool failed = false;
            RuleResult failure_result = RuleResult::mismatch();
            std::size_t step_index = 0;

            auto execute_step = [&](const auto& rule) -> bool {
                if (failed)
                {
                    return false;
                }
                
                if (skip_trivia_ && step_index > 0)
                {
                    Trivia::skip_all(cursor);
                }

                auto step_result = rule(cursor);

                if (step_result.is_mismatch())
                {
                    failed = true;
                    failure_result = RuleResult::mismatch();
                    return false;
                }

                if (step_result.is_failure())
                {
                    failed = true;
                    failure_result = std::move(step_result);
                    return false;
                }

                parent_node.children.push_back(std::move(step_result.node()));
                ++step_index;
                return true;
            };

            std::apply([&](const auto&... r) {
                (execute_step(r) && ...);
            }, rules_);

            if (failed)
            {
                cursor.restore(cp);
                return failure_result;
            }

            parent_node.range = cursor.range_since(cp);
            parent_node.text = cursor.slice_since(cp);

            return RuleResult::success(std::move(parent_node));
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        std::tuple<Rules...> rules_;
        bool skip_trivia_{true};
    };
} // namespace NodeSystem::lsp::rules::combinators