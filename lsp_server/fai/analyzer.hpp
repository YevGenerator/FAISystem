#pragma once

#include <string_view>
#include <vector>
#include <format>

#include "../lsp_core/protocol/completion/completion_item.hpp"
#include "../lsp_core/rules/cursor.hpp"
#include "../lsp_core/rules/combinators/trivia.hpp"
#include "../lsp_core/protocol/diagnostic/diagnostic.hpp"
#include "../lsp_core/protocol/documents/document_item.hpp"
#include "syntax/let_declaration.hpp"
#include "syntax/keywords.hpp"
#include "syntax/signs.hpp"

namespace NodeSystem::lsp::fai
{
    class Analyzer
    {
    public:
        static std::vector<protocol::Diagnostic> analyze(const protocol::documents::DocumentItem& doc)
        {
            std::vector<protocol::Diagnostic> diagnostics;
            rules::SourceCursor cursor(doc.text);
            const syntax::LetDeclarationRule let_rule;

            while (!cursor.is_eof())
            {
                rules::combinators::Trivia::skip_all(cursor);
                if (cursor.is_eof()) break;

                const auto cmd_start_cp = cursor.mark();
                auto res = let_rule(cursor);

                if (res.is_success())
                {
                    rules::combinators::Trivia::skip_horizontal(cursor);
                    if (!cursor.is_eof() && cursor.peek() != '\n' && cursor.peek() != '\r')
                    {
                        const auto err_cp = cursor.mark();
                        skip_to_next_line(cursor);
                        diagnostics.push_back(protocol::Diagnostic{
                            .range = cursor.range_since(err_cp),
                            .severity = protocol::Diagnostic::Severity::Error,
                            .message = "Синтаксична помилка: неочікувані символи після завершення команди"
                        });
                    }
                }
                else if (res.is_failure())
                {
                    diagnostics.push_back(std::move(res.diagnostic()));
                    skip_to_next_line(cursor);
                }
                else
                {
                    cursor.restore(cmd_start_cp);
                    const auto token_cp = cursor.mark();
                    syntax::Identifier id_scanner;
                    auto id_res = id_scanner(cursor);

                    if (id_res.is_success())
                    {
                        diagnostics.push_back(protocol::Diagnostic{
                            .range = cursor.range_since(token_cp),
                            .severity = protocol::Diagnostic::Severity::Error,
                            .message = std::format("Невідома команда '{}'", id_res.node().text)
                        });
                    }
                    else
                    {
                        cursor.advance();
                        diagnostics.push_back(protocol::Diagnostic{
                            .range = cursor.range_since(token_cp),
                            .severity = protocol::Diagnostic::Severity::Error,
                            .message = "Синтаксична помилка: неочікуваний символ на початку команди"
                        });
                    }
                    skip_to_next_line(cursor);
                }
            }
            return diagnostics;
        }

        static std::vector<std::uint32_t> highlight(const protocol::documents::DocumentItem& doc)
        {
            protocol::SemanticTokensBuilder builder;
            rules::SourceCursor cursor(doc.text);
            const syntax::LetDeclarationRule let_rule;

            while (!cursor.is_eof())
            {
                rules::combinators::Trivia::skip_all(cursor);
                if (cursor.is_eof()) break;

                auto res = let_rule(cursor);
                if (res.is_success())
                {
                    const auto& node = res.node();
                    builder.add_token(node.range.start, 3, protocol::SemanticTokenType::Keyword);

                    if (const auto* type_node = node.find_child("type"))
                    {
                        builder.add_token(type_node->range.start, type_node->text.size(), protocol::SemanticTokenType::Type);
                    }
                    if (const auto* name_node = node.find_child("name"))
                    {
                        builder.add_token(name_node->range.start, name_node->text.size(), protocol::SemanticTokenType::Variable);
                    }
                    if (const auto* val_node = node.find_child("value"))
                    {
                        builder.add_token(val_node->range.start, val_node->text.size(), protocol::SemanticTokenType::Number);
                    }
                }
                else
                {
                    skip_to_next_line(cursor);
                }
            }

            return builder.data();
        }
        
        static std::vector<protocol::CompletionItem> complete(
            const protocol::documents::DocumentItem& doc,
            protocol::Position pos)
        {
            std::vector<protocol::CompletionItem> items;
            std::string_view text = doc.text;
            std::size_t current_line = 0;
            std::size_t line_start = 0;
            std::size_t line_end = text.size();

            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (current_line == pos.line)
                {
                    line_start = i;
                    while (i < text.size() && text[i] != '\n' && text[i] != '\r') ++i;
                    line_end = i;
                    break;
                }
                if (text[i] == '\n') ++current_line;
            }

            std::string_view line_prefix = text.substr(
                line_start,
                (line_start + pos.character <= line_end) ? pos.character : line_end - line_start
            );

            while (!line_prefix.empty() && (line_prefix.front() == ' ' || line_prefix.front() == '\t'))
            {
                line_prefix.remove_prefix(1);
            }

            const protocol::Command retrigger_cmd{
                .title = "Trigger Suggest",
                .command = "editor.action.triggerSuggest"
            };

            if (line_prefix.empty() || syntax::keywords::let_cmd.starts_with(line_prefix))
            {
                items.push_back(protocol::CompletionItem{
                    .label = std::string(syntax::keywords::let_cmd),
                    .kind = protocol::CompletionItemKind::Keyword,
                    .detail = "Оголошення змінної",
                    .documentation = "Синтаксис: let <type> <name> = <value>",
                    .insert_text = "let ",
                    .command = retrigger_cmd
                });
            }
            else if (line_prefix.starts_with("let "))
            {
                std::string_view after_let = line_prefix.substr(4);
                if (after_let.empty() || std::string_view("uint8").starts_with(after_let))
                {
                    items.push_back(protocol::CompletionItem{
                        .label = "uint8",
                        .kind = protocol::CompletionItemKind::Class,
                        .detail = "Ціле число [0..255]",
                        .documentation = "8-бітне беззнакове число",
                        .insert_text = "uint8 ",
                        .command = retrigger_cmd
                    });
                }
            }

            return items;
        }

    private:
        static void skip_to_next_line(rules::SourceCursor& cursor) noexcept
        {
            while (!cursor.is_eof())
            {
                const auto ch = cursor.peek();
                if (!ch.has_value() || *ch == '\n' || *ch == '\r') break;
                cursor.advance();
            }
        }
    };
} // namespace NodeSystem::lsp::fai