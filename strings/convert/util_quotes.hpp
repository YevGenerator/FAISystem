#pragma once
#include <format>

namespace NodeSystem::strings::convert
{
    struct UtilQuotes
    {
        static auto in_quotes(auto value)
        {
            return std::format("\"{}\"", value);
        }

        static auto from_quotes(std::string_view str)
        {
            if (str.size() >= 2 && str.front() == '"' && str.back() == '"')
            {
                return str.substr(1, str.size() - 2);
            }
            return str;
        }
    };
}
