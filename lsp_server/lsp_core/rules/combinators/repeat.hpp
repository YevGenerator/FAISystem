#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "../cursor.hpp"
#include "../rule.hpp"
#include "../rule_result.hpp"
#include "../syntax_node.hpp"
#include "trivia.hpp"

namespace NodeSystem::lsp::rules::combinators
{
    struct NoSeparator
    {
    };

    template <Rule ElementRule, typename SeparatorRule = NoSeparator>
    class Repeat
    {
    public:
        explicit Repeat(ElementRule element, std::size_t min_count = 0, bool auto_skip_trivia = true)
            requires std::is_same_v<SeparatorRule, NoSeparator>
            : element_(std::move(element)),
              min_count_(min_count),
              skip_trivia_(auto_skip_trivia)
        {
        }

        Repeat(ElementRule element, SeparatorRule separator, std::size_t min_count = 0, bool auto_skip_trivia = true)
            requires (!std::is_same_v<SeparatorRule, NoSeparator>)
            : element_(std::move(element)),
              separator_(std::move(separator)),
              min_count_(min_count),
              skip_trivia_(auto_skip_trivia)
        {
        }

        [[nodiscard]] RuleResult match(SourceCursor& cursor) const
        {
            const auto start_cp = cursor.mark();

            SyntaxNode result_node{
                .tag = "repeat",
                .text = {},
                .range = cursor.range_since(start_cp),
                .children = {}
            };

            std::size_t matched_count = 0;

            if constexpr (std::is_same_v<SeparatorRule, NoSeparator>)
            {
                while (!cursor.is_eof())
                {
                    if (skip_trivia_)
                    {
                        Trivia::skip_all(cursor);
                    }

                    const auto iter_cp = cursor.mark();
                    auto res = element_(cursor);

                    if (res.is_mismatch())
                    {
                        cursor.restore(iter_cp);
                        break;
                    }

                    if (res.is_failure())
                    {
                        return res;
                    }

                    if (cursor.current_offset() == iter_cp.offset)
                    {
                        break;
                    }

                    result_node.children.push_back(std::move(res.node()));
                    ++matched_count;
                }
            }
            else
            {
                if (skip_trivia_)
                {
                    Trivia::skip_all(cursor);
                }

                const auto first_cp = cursor.mark();
                auto first_res = element_(cursor);

                if (first_res.is_failure())
                {
                    return first_res;
                }

                if (first_res.is_success())
                {
                    result_node.children.push_back(std::move(first_res.node()));
                    ++matched_count;

                    while (!cursor.is_eof())
                    {
                        const auto sep_cp = cursor.mark();

                        if (skip_trivia_)
                        {
                            Trivia::skip_all(cursor);
                        }

                        auto sep_res = separator_(cursor);
                        if (sep_res.is_mismatch())
                        {
                            cursor.restore(sep_cp);
                            break;
                        }

                        if (sep_res.is_failure())
                        {
                            return sep_res;
                        }

                        if (skip_trivia_)
                        {
                            Trivia::skip_all(cursor);
                        }

                        auto elem_res = element_(cursor);
                        if (elem_res.is_mismatch())
                        {
                            cursor.restore(sep_cp);
                            break;
                        }

                        if (elem_res.is_failure())
                        {
                            return elem_res;
                        }

                        if (cursor.current_offset() == sep_cp.offset)
                        {
                            break;
                        }

                        result_node.children.push_back(std::move(elem_res.node()));
                        ++matched_count;
                    }
                }
                else
                {
                    cursor.restore(first_cp);
                }
            }

            if (matched_count < min_count_)
            {
                cursor.restore(start_cp);
                return RuleResult::mismatch();
            }

            result_node.range = cursor.range_since(start_cp);
            result_node.text = cursor.slice_since(start_cp);

            return RuleResult::success(std::move(result_node));
        }

        RuleResult operator()(SourceCursor& cursor) const
        {
            return match(cursor);
        }

    private:
        ElementRule element_;
        [[no_unique_address]] SeparatorRule separator_{};
        std::size_t min_count_{0};
        bool skip_trivia_{true};
    };
    
    template <Rule ElementRule>
    Repeat(ElementRule) -> Repeat<ElementRule, NoSeparator>;

    template <Rule ElementRule>
    Repeat(ElementRule, std::size_t) -> Repeat<ElementRule, NoSeparator>;

    template <Rule ElementRule, Rule SeparatorRule>
    Repeat(ElementRule, SeparatorRule) -> Repeat<ElementRule, SeparatorRule>;

    template <Rule ElementRule, Rule SeparatorRule>
    Repeat(ElementRule, SeparatorRule, std::size_t) -> Repeat<ElementRule, SeparatorRule>;
} // namespace NodeSystem::lsp::rules::combinators