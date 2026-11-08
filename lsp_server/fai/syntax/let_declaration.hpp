#pragma once

#include "identifier.hpp"
#include "../../lsp_core/rules/cursor.hpp"
#include "../../lsp_core/rules/rule_result.hpp"
#include "../../lsp_core/rules/rule.hpp"
#include "../../lsp_core/rules/combinators/exact.hpp"
#include "../../lsp_core/rules/combinators/sequence.hpp"
#include "../../lsp_core/rules/combinators/tag.hpp"
#include "../../lsp_core/rules/combinators/expect.hpp"
#include "signs.hpp"
#include "keywords.hpp"
#include "../types/uint8.hpp"

namespace NodeSystem::lsp::fai::syntax
{
    using namespace rules::combinators;

    class LetDeclarationRule
    {
    public:
        LetDeclarationRule() : rule_(Tag(
            "let_declaration",
            Sequence(
                Exact(keywords::let_cmd),
                Expect(
                    Tag("type", Exact("uint8")),
                    "Очікувався тип змінної (наприклад, 'uint8')"
                ),
                Expect(
                    Tag("name", Identifier{}),
                    "Очікувалась назва змінної"
                ),
                Expect(
                    Exact(signs::assign),
                    "Очікувався знак присвоєння '='"
                ),
                Expect(
                    Tag("value", types::Uint8Rule{}),
                    "Очікувалось значення для типу uint8 [0..255]"
                ))))
        {
        }

        [[nodiscard]] rules::RuleResult operator()(rules::SourceCursor& cursor) const
        {
            return rule_(cursor);
        }

    private:
        rules::AnyRule rule_;
    };
} // namespace NodeSystem::lsp::fai::syntax
