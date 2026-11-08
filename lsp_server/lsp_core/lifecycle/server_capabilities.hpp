#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "../json_fields.hpp"
#include "../protocol/semantic_tokens/semantic_tokens.hpp"

namespace NodeSystem::lsp::lifecycle
{
    struct ServerInfo
    {
        std::string name;
        std::optional<std::string> version{std::nullopt};

        [[nodiscard]] nlohmann::json to_json() const
        {
            nlohmann::json j{
                {json_names::name, name}
            };
            if (version.has_value())
            {
                j[json_names::version] = *version;
            }
            return j;
        }
    };

    struct CompletionOptions
    {
        bool resolveProvider{false};
        std::vector<std::string> triggerCharacters{" ", "="};

        [[nodiscard]] nlohmann::json to_json() const
        {
            nlohmann::json j{
                {json_names::resolveProvider, resolveProvider}
            };
            if (!triggerCharacters.empty())
            {
                j[json_names::triggerCharacters] = triggerCharacters;
            }
            return j;
        }
    };

    struct SemanticTokensOptions
    {
        protocol::SemanticTokensLegend legend;
        bool full{true};

        [[nodiscard]] nlohmann::json to_json() const
        {
            return {
                {"legend", protocol::SemanticTokensLegend::to_json()},
                {"full", full}
            };
        }
    };

    struct ServerCapabilities
    {
        using enum_t = std::uint8_t;

        enum class TextDocumentSyncKind : enum_t
        {
            None = 0,
            Full = 1,
            Incremental = 2
        };

        TextDocumentSyncKind textDocumentSync{TextDocumentSyncKind::Full};
        std::optional<CompletionOptions> completionProvider{CompletionOptions{}};
        std::optional<SemanticTokensOptions> semanticTokensProvider{SemanticTokensOptions{}};

        [[nodiscard]]
        nlohmann::json to_json() const
        {
            nlohmann::json j{
                {json_names::textDocumentSync, static_cast<enum_t>(textDocumentSync)}
            };
            if (completionProvider.has_value())
            {
                j[json_names::completionProvider] = completionProvider->to_json();
            }

            if (semanticTokensProvider.has_value())
            {
                j["semanticTokensProvider"] = semanticTokensProvider->to_json();
            }
            return j;
        }
    };

    struct InitializeResult
    {
        ServerCapabilities capabilities;
        std::optional<ServerInfo> serverInfo{std::nullopt};

        [[nodiscard]] nlohmann::json to_json() const
        {
            nlohmann::json j{
                {json_names::capabilities, capabilities.to_json()}
            };
            if (serverInfo.has_value())
            {
                j[json_names::serverInfo] = serverInfo->to_json();
            }
            return j;
        }

        constexpr static InitializeResult Default()
        {
            InitializeResult result{};
            result.capabilities.textDocumentSync = ServerCapabilities::TextDocumentSyncKind::Full;
            result.serverInfo = ServerInfo{
                .name = "NodeSystem-LSP",
                .version = "1.0.0"
            };
            return result;
        }
    };
} // namespace NodeSystem::lsp::lifecycle
