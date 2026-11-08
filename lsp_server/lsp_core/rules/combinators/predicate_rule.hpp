#pragma once

#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"
#include "../../protocol/diagnostic/diagnostic.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule InnerRule, typename Validator>
    class PredicateRule
    {
    public:
        PredicateRule(InnerRule rule, Validator validator)
            : rule_(std::move(rule)), validator_(std::move(validator))
        {
        }

        [[nodiscard]] RuleResult match(SourceCursor& cursor) const
        {
            auto res = rule_(cursor);

            if (!res.is_success())
            {
                return res;
            }

            const auto& node = res.node();

            if constexpr (std::is_invocable_r_v<std::optional<std::string>, Validator, const SyntaxNode&>)
            {
                if (auto error_msg = validator_(node); error_msg.has_value())
                {
                    return RuleResult::failure(protocol::Diagnostic{
                        .range = node.range,
                        .severity = protocol::Diagnostic::Severity::Error,
                        .message = std::move(*error_msg)
                    });
                }
            }
            else if constexpr (std::is_invocable_r_v<std::optional<protocol::Diagnostic>, Validator, const SyntaxNode&>)
            {
                if (auto diag = validator_(node); diag.has_value())
                {
                    return RuleResult::failure(std::move(*diag));
                }
            }
            else if constexpr (std::is_invocable_r_v<bool, Validator, const SyntaxNode&>)
            {
                if (!validator_(node))
                {
                    return RuleResult::failure(protocol::Diagnostic{
                        .range = node.range,
                        .severity = protocol::Diagnostic::Severity::Error,
                        .message = "Семантична помилка: значення не відповідає обмеженням домену"
                    });
                }
            }

            return res;
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        InnerRule rule_;
        Validator validator_;
    };

    template <Rule InnerRule, typename Validator>
    PredicateRule(InnerRule, Validator) -> PredicateRule<InnerRule, Validator>;
} // namespace NodeSystem::lsp::rules::combinators
