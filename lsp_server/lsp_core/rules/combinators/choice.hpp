#pragma once

#include <tuple>
#include <utility>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule... Rules>
    requires (sizeof...(Rules) > 0)
    class Choice
    {
    public:
        explicit Choice(Rules... rules)
            : rules_(std::move(rules)...)
        {
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const
        {
            const auto cp = cursor.mark();
            RuleResult final_result = RuleResult::mismatch();
            bool matched = false;

            auto try_rule = [&](const auto& rule) -> bool {
                cursor.restore(cp);

                auto res = rule(cursor);

                if (res.is_success())
                {
                    matched = true;
                    final_result = std::move(res);
                    return true;
                }

                if (res.is_failure())
                {
                    matched = true;
                    final_result = std::move(res);
                    return true;
                }

                return false;
            };

            std::apply([&](const auto&... r) {
                (try_rule(r) || ...);
            }, rules_);

            if (!matched)
            {
                cursor.restore(cp);
                return RuleResult::mismatch();
            }

            return final_result;
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        std::tuple<Rules...> rules_;
    };
} // namespace NodeSystem::lsp::rules::combinators