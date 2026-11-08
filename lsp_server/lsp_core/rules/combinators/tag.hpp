#pragma once

#include <string>
#include <utility>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    template <Rule InnerRule>
    class Tag
    {
    public:
        Tag(std::string tag_name, InnerRule rule)
            : tag_name_(std::move(tag_name)), rule_(std::move(rule))
        {
        }

        [[nodiscard]]
        const std::string& tag_name() const noexcept
        {
            return tag_name_;
        }

        [[nodiscard]]
        RuleResult match(SourceCursor& cursor) const
        {
            RuleResult result = rule_(cursor);
            
            if (result.is_success())
            {
                result.node().tag = tag_name_;
            }

            return result;
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        std::string tag_name_;
        InnerRule rule_;
    };
} // namespace NodeSystem::lsp::rules::combinators