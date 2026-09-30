#pragma once
#include <vector>

#include "../type_parser.hpp"
#include "../../preparser.hpp"

namespace NodeSystem::strings::convert
{
    template <IsAnyStringConverter TItemConverter>
    struct ListConverter
    {
        using item_t = TItemConverter::target_t;
        using target_t = std::vector<item_t>;

        static std::optional<target_t> fromString(std::string_view str, const ParserContext& context)
        {
            if (auto val = context.resolve<target_t>(str))
            {
                return val;
            }

            auto innerContent = Preparser::unwrapList(str);
            if (!innerContent)
            {
                return std::nullopt;
            }

            target_t result;

            while (true)
            {
                std::string_view itemToken = Preparser::consumeNextValue(*innerContent);
                if (itemToken.empty()) break;

                auto parsedItem = TypeParser::resolveOrParse<TItemConverter>(itemToken, context);

                if (!parsedItem) return std::nullopt;

                result.push_back(std::move(*parsedItem));
            }

            return result;
        }

        static std::string toString(const target_t& list)
        {
            std::string out = std::string(Keywords::Chars::ListStart);
            for (size_t i = 0; i < list.size(); ++i)
            {
                if (i > 0) out += ' ';
                out += TypeParser::toString<TItemConverter>(list[i]);
            }
            out += Keywords::Chars::ListEnd;
            return out;
        }

        static std::string toStringShort(const target_t& list)
        {
            std::string out = std::string(Keywords::Chars::ListStart);
            for (size_t i = 0; i < list.size(); ++i)
            {
                if (i > 0) out += ' ';
                out += TypeParser::toStringShort<TItemConverter>(list[i]);
            }
            out += Keywords::Chars::ListEnd;
            return out;
        }
    };
}
