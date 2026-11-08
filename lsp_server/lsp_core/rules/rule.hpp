#pragma once

#include <concepts>
#include <functional>
#include "cursor.hpp"
#include "rule_result.hpp"

namespace NodeSystem::lsp::rules
{
    template <typename T>
    concept Rule = requires(T rule, SourceCursor& cursor) {
        { rule(cursor) } -> std::same_as<RuleResult>;
    };

    using AnyRule = std::function<RuleResult(SourceCursor&)>;
}