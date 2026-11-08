#pragma once

#include <variant>
#include <utility>

#include "../protocol/diagnostic/diagnostic.hpp"
#include "syntax_node.hpp"

namespace NodeSystem::lsp::rules
{
    struct RuleMismatch
    {
    };

    class RuleResult
    {
    public:
        using ValueType = std::variant<SyntaxNode, RuleMismatch, protocol::Diagnostic>;

        static RuleResult success(SyntaxNode node)
        {
            return RuleResult(std::move(node));
        }

        static RuleResult mismatch() noexcept
        {
            return RuleResult(RuleMismatch{});
        }

        static RuleResult failure(protocol::Diagnostic diagnostic)
        {
            return RuleResult(std::move(diagnostic));
        }

        [[nodiscard]]
        bool is_success() const noexcept
        {
            return std::holds_alternative<SyntaxNode>(storage_);
        }

        [[nodiscard]]
        bool is_mismatch() const noexcept
        {
            return std::holds_alternative<RuleMismatch>(storage_);
        }

        [[nodiscard]]
        bool is_failure() const noexcept
        {
            return std::holds_alternative<protocol::Diagnostic>(storage_);
        }

        [[nodiscard]]
        SyntaxNode& node() &
        {
            return std::get<SyntaxNode>(storage_);
        }

        [[nodiscard]]
        const SyntaxNode& node() const &
        {
            return std::get<SyntaxNode>(storage_);
        }

        [[nodiscard]]
        SyntaxNode&& node() &&
        {
            return std::get<SyntaxNode>(std::move(storage_));
        }

        [[nodiscard]]
        protocol::Diagnostic& diagnostic() &
        {
            return std::get<protocol::Diagnostic>(storage_);
        }

        [[nodiscard]]
        const protocol::Diagnostic& diagnostic() const &
        {
            return std::get<protocol::Diagnostic>(storage_);
        }

        [[nodiscard]]
        protocol::Diagnostic&& diagnostic() &&
        {
            return std::get<protocol::Diagnostic>(std::move(storage_));
        }

    private:
        explicit RuleResult(ValueType val)
            : storage_(std::move(val))
        {
        }

        ValueType storage_;
    };
} // namespace NodeSystem::lsp::rules