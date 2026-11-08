#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "completion_item_kind.hpp"
#include "../../json_fields.hpp"

namespace NodeSystem::lsp::protocol
{
    struct Command
    {
        std::string title;
        std::string command;

        [[nodiscard]] nlohmann::json to_json() const
        {
            return {
                {"title", title},
                {"command", command}
            };
        }
    };

    struct CompletionItem
    {
        std::string label;
        CompletionItemKind kind{CompletionItemKind::Text};
        std::optional<std::string> detail{std::nullopt};
        std::optional<std::string> documentation{std::nullopt};
        std::optional<std::string> insert_text{std::nullopt};
        std::optional<Command> command{std::nullopt};

        [[nodiscard]]
        nlohmann::json to_json() const
        {
            nlohmann::json j{
                {json_names::label, label},
                {json_names::kind, static_cast<std::uint8_t>(kind)}
            };

            if (detail.has_value())
            {
                j[json_names::detail] = *detail;
            }
            if (documentation.has_value())
            {
                j[json_names::documentation] = *documentation;
            }
            if (insert_text.has_value())
            {
                j[json_names::insertText] = *insert_text;
            }

            if (command.has_value()) j["command"] = command->to_json();

            return j;
        }
    };
} // namespace NodeSystem::lsp::protocol
