#pragma once

#include <string_view>

namespace NodeSystem::lsp::fai::syntax::signs
{
    constexpr std::string_view assign = "=";

    constexpr std::string_view open_node = "<";
    constexpr std::string_view close_node = ">";

    constexpr std::string_view open_array = "[";
    constexpr std::string_view close_array = "]";

    constexpr std::string_view plus = "+";
    constexpr std::string_view minus = "-";
} // namespace NodeSystem::fai::syntax::signs