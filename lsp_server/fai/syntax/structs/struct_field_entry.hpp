#pragma once

#include <optional>
#include <string_view>

#include "../identifier.hpp"
#include "../raw_value_token.hpp"
#include "../../../lsp_core/rules/cursor.hpp"
#include "../../../lsp_core/rules/rule_result.hpp"
#include "../../../lsp_core/rules/syntax_node.hpp"
#include "../../../lsp_core/rules/combinators/trivia.hpp"
#include "../signs.hpp"


namespace NodeSystem::lsp::fai::syntax::structs
{
    class StructFieldEntry
    {
    public:
        [[nodiscard]] rules::RuleResult operator()(rules::SourceCursor& cursor) const
        {
            const auto start_cp = cursor.mark();
            
            Identifier ident_scanner;
            auto ident_res = ident_scanner(cursor);

            if (ident_res.is_success())
            {
                const auto after_ident_cp = cursor.mark();
                rules::combinators::Trivia::skip_all(cursor);

                if (cursor.starts_with(signs::assign))
                {
                    cursor.consume_prefix(signs::assign);
                    rules::combinators::Trivia::skip_all(cursor);

                    RawValueToken value_scanner;
                    auto val_res = value_scanner(cursor);

                    if (val_res.is_success())
                    {
                        rules::SyntaxNode field_node{
                            .tag = "named_field",
                            .text = cursor.slice_since(start_cp),
                            .range = cursor.range_since(start_cp),
                            .children = {
                                std::move(ident_res.value()),
                                std::move(val_res.value())
                            }
                        };
                        return rules::RuleResult::success(std::move(field_node));
                    }
                }

                cursor.restore(after_ident_cp);
            }

            cursor.restore(start_cp);
            RawValueToken value_scanner;
            auto val_res = value_scanner(cursor);

            if (val_res.is_success())
            {
                rules::SyntaxNode pos_node{
                    .tag = "positional_field",
                    .text = cursor.slice_since(start_cp),
                    .range = cursor.range_since(start_cp),
                    .children = {
                        std::move(val_res.value())
                    }
                };
                return rules::RuleResult::success(std::move(pos_node));
            }

            cursor.restore(start_cp);
            return rules::RuleResult::mismatch();
        }
    };
} // namespace NodeSystem::lsp::fai::syntax::structs