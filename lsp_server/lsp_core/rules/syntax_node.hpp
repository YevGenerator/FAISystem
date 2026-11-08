#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>

#include "../protocol/range.hpp"

namespace NodeSystem::lsp::rules
{
    struct SyntaxNode
    {
        std::string tag;
        std::string_view text;
        protocol::Range range;
        std::vector<SyntaxNode> children{};

        [[nodiscard]]
        constexpr bool has_children() const noexcept
        {
            return !children.empty();
        }

        [[nodiscard]]
        const SyntaxNode* find_child(std::string_view target_tag) const noexcept
        {
            for (const auto& child : children)
            {
                if (child.tag == target_tag)
                {
                    return &child;
                }
            }
            return nullptr;
        }

        [[nodiscard]]
        std::vector<const SyntaxNode*> find_all_children(std::string_view target_tag) const
        {
            std::vector<const SyntaxNode*> results;
            for (const auto& child : children)
            {
                if (child.tag == target_tag)
                {
                    results.push_back(&child);
                }
            }
            return results;
        }
    };
} // namespace NodeSystem::lsp::rules