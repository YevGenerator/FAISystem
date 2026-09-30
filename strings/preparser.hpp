#pragma once
#include <string_view>
#include <optional>
#include <cctype>
#include "cmd_keywords.hpp"

namespace NodeSystem::strings
{
    struct Preparser
    {
        static bool IsSpace(auto value)
        {
            return std::isspace(static_cast<unsigned char>(value));
        }

        static void consumeWhitespaceAndComments(std::string_view& str)
        {
            while (!str.empty())
            {
                if (IsSpace(str.front()))
                {
                    str.remove_prefix(1);
                }
                else if (str.starts_with(Keywords::Chars::Comment))
                {
                    const auto newline_pos = str.find('\n');
                    if (newline_pos == std::string_view::npos)
                    {
                        str = {};
                    }
                    else
                    {
                        str.remove_prefix(newline_pos + 1);
                    }
                }
                else
                {
                    break;
                }
            }
        }

        static std::optional<std::string_view> unwrapBlock(
            std::string_view& str, std::string_view openMarker, std::string_view closeMarker)
        {
            consumeWhitespaceAndComments(str);
            if (str.starts_with(openMarker) && str.ends_with(closeMarker))
            {
                return str.substr(openMarker.size(),
                                  str.size() - openMarker.size() - closeMarker.size());
            }
            return std::nullopt;
        }

        static std::optional<std::string_view> unwrapStruct(std::string_view str)
        {
            return unwrapBlock(str, Keywords::Chars::StructOpen, Keywords::Chars::StructClose);
        }

        static std::optional<std::string_view> unwrapList(std::string_view str)
        {
            return unwrapBlock(str, Keywords::Chars::ListStart, Keywords::Chars::ListEnd);
        }

        static std::optional<std::string_view> extractBlock(
            std::string_view& str, std::string_view openMarker, std::string_view closeMarker)
        {
            if (!str.starts_with(openMarker)) return std::nullopt;
            size_t nestingLevel = 0;
            size_t i = 0;
            while (i < str.size())
            {
                std::string_view current = str.substr(i);
                if (current.starts_with(openMarker))
                {
                    nestingLevel++;
                    i += openMarker.size();
                }
                else if (current.starts_with(closeMarker))
                {
                    nestingLevel--;
                    if (nestingLevel == 0)
                    {
                        std::string_view block = str.substr(openMarker.size(), i - openMarker.size());
                        str.remove_prefix(i + closeMarker.size());
                        return block;
                    }
                    i += closeMarker.size();
                }
                else
                {
                    i++;
                }
            }
            return std::nullopt;
        }

        static std::string_view consumeBlock(std::string_view& str,
                                             std::string_view openMarker, std::string_view closeMarker)
        {
            if (auto block = extractBlock(str, openMarker, closeMarker))
            {
                return {
                    block->data() - openMarker.size(),
                    block->size() + openMarker.size() + closeMarker.size()
                };
            }
            return {};
        }

        static auto consumeStruct(std::string_view& str)
        {
            return consumeBlock(str, Keywords::Chars::StructOpen, Keywords::Chars::StructClose);
        }

        static auto consumeList(std::string_view& str)
        {
            return consumeBlock(str, Keywords::Chars::ListStart, Keywords::Chars::ListEnd);
        }

        static std::string_view consumeNextToken(std::string_view& str)
        {
            consumeWhitespaceAndComments(str);
            if (str.empty()) return {};

            size_t len = 0;

            if (str.starts_with(Keywords::Chars::ListStart))
            {
                auto withoutStart = str.substr(Keywords::Chars::ListStart.size());
                if (withoutStart.starts_with(Keywords::Chars::ListEnd))
                {
                    len = Keywords::Chars::ListStart.size() + Keywords::Chars::ListEnd.size();
                }
            }

            while (len < str.size())
            {
                if (IsSpace(str[len])) break;
                auto current = str.substr(len);
                if (current.starts_with(Keywords::Chars::StructOpen) ||
                    current.starts_with(Keywords::Chars::StructClose) ||
                    current.starts_with(Keywords::Chars::ListStart) ||
                    current.starts_with(Keywords::Chars::ListEnd) ||
                    current.starts_with(Keywords::Chars::Comment))
                {
                    break;
                }
                len++;
            }

            if (len == 0)
            {
                len = 1;
            }

            auto token = str.substr(0, len);
            str.remove_prefix(len);
            return token;
        }

        static std::string_view consumeNextValue(std::string_view& str)
        {
            consumeWhitespaceAndComments(str);
            if (str.empty()) return {};
            if (str.starts_with(Keywords::Chars::StructOpen))
            {
                return consumeStruct(str);
            }
            if (str.starts_with(Keywords::Chars::ListStart))
            {
                return consumeList(str);
            }
            return consumeNextToken(str);
        }

        static std::optional<std::pair<std::string_view, std::string_view>> splitNamedField(std::string_view token)
        {
            size_t eqPos = 0;
            while (eqPos < token.size())
            {
                auto current = token.substr(eqPos);
                if (current.starts_with(Keywords::Chars::StructOpen) ||
                    current.starts_with(Keywords::Chars::ListStart))
                {
                    break;
                }
                if (current.starts_with(Keywords::Chars::AssignParam))
                {
                    return std::pair{
                        token.substr(0, eqPos),
                        token.substr(eqPos + Keywords::Chars::AssignParam.size())
                    };
                }
                eqPos++;
            }
            return std::nullopt;
        }
    };
}