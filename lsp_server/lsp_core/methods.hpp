#pragma once
#include <string_view>
namespace NodeSystem::lsp::methods
{
    static constexpr std::string_view publish = "textDocument/publishDiagnostics";
    static constexpr std::string_view open = "textDocument/didOpen";
    static constexpr std::string_view change = "textDocument/didChange";
    static constexpr std::string_view close = "textDocument/didClose";
    static constexpr std::string_view completion = "textDocument/completion";

    static constexpr std::string_view init = "initialize";
    static constexpr std::string_view inited = "initialized";
    static constexpr std::string_view shutdown = "shutdown";
    static constexpr std::string_view exit = "exit";

}
