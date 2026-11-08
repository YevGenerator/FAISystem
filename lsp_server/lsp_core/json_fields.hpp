#pragma once
#include <string_view>
namespace NodeSystem::lsp::json_names
{
    constexpr static std::string_view capabilities = "capabilities";
    constexpr static std::string_view character = "character";
    constexpr static std::string_view code = "code";
    constexpr static std::string_view contentChanges = "contentChanges";
    constexpr static std::string_view completionProvider = "completionProvider";

    constexpr static std::string_view detail = "detail";
    constexpr static std::string_view diagnostics = "diagnostics";
    constexpr static std::string_view documentation = "documentation";

    constexpr static std::string_view end = "end";
    constexpr static std::string_view error_data = "data";
    constexpr static std::string_view error = "error";

    constexpr static std::string_view header = "jsonrpc";
    constexpr static std::string_view header_value = "2.0";

    constexpr static std::string_view id = "id";
    constexpr static std::string_view insertText = "insertText";

    constexpr static std::string_view kind = "kind";

    constexpr static std::string_view line = "line";
    constexpr static std::string_view label = "label";
    constexpr static std::string_view language = "languageId";

    constexpr static std::string_view method = "method";
    constexpr static std::string_view message = "message";

    constexpr static std::string_view name = "name";

    constexpr static std::string_view params = "params";
    constexpr static std::string_view position = "position";

    constexpr static std::string_view range = "range";
    constexpr static std::string_view result = "result";
    constexpr static std::string_view resolveProvider = "resolverProvider";

    constexpr static std::string_view serverInfo = "serverInfo";
    constexpr static std::string_view severity = "severity";
    constexpr static std::string_view source = "source";
    constexpr static std::string_view start = "start";

    constexpr static std::string_view textDocumentSync = "textDocumentSync";
    constexpr static std::string_view textDocument = "textDocument";
    constexpr static std::string_view text = "text";
    constexpr static std::string_view triggerCharacters = "triggerCharacters";

    constexpr static std::string_view uri = "uri";

    constexpr static std::string_view version = "version";
}
