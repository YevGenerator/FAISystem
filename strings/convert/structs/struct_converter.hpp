#pragma once
#include <format>

#include "../../preparser.hpp"
#include "../../ParserContext.hpp"
#include "../type_parser.hpp"

namespace NodeSystem::strings::convert
{
    template <const auto& MetaScheme>
    struct StructConverter
    {
        using scheme_t = std::remove_cvref_t<decltype(MetaScheme)>;
        using target_t = scheme_t::target_t;

        static std::optional<target_t> fromString(std::string_view str, const ParserContext& context)
        {
            if (auto val = context.resolve<target_t>(str))
            {
                return val;
            }

            auto inner = Preparser::unwrapStruct(str);
            if (!inner) return std::nullopt;

            target_t result{};
            size_t positionalIndex = 0;

            while (true)
            {
                std::string_view token = Preparser::consumeNextValue(*inner);
                if (token.empty()) break;

                auto namedField = Preparser::splitNamedField(token);
                std::string_view nameStr = namedField ? namedField->first : "";
                std::string_view valStr  = namedField ? namedField->second : token;

                if (namedField && valStr.empty())
                {
                    valStr = Preparser::consumeNextValue(*inner);
                }

                bool matched = false;

                std::apply([&](const auto&... fields)
                {
                    size_t currentIndex = 0;
                    auto try_match = [&]<typename TField>(const TField& field)
                    {
                        if (matched) return;

                        if ((namedField && nameStr == field.name) || (!namedField && positionalIndex == currentIndex))
                        {
                            using FieldConverter = TField::converter_t;
                            auto parsed = TypeParser::resolveOrParse<FieldConverter>(valStr, context);
                            if (parsed)
                            {
                                field.set(result, *parsed);
                                matched = true;
                            }
                        }
                        currentIndex++;
                    };

                    (try_match(fields), ...);
                }, MetaScheme.fields);

                if (!matched) return std::nullopt;
                positionalIndex++;
            }

            return result;
        }

        static std::string toString(const target_t& val)
        {
            auto out = std::string(Keywords::Chars::StructOpen);
            bool first = true;

            std::apply([&](const auto&... fields)
            {
                auto stringify_field = [&]<typename TField>(const TField& field)
                {
                    if (!first) out += ' ';
                    out += std::format("{}{}{}",
                                       field.name,
                                       Keywords::Chars::AssignParam,
                                       TypeParser::toString<typename TField::converter_t>(field.get(val)));
                    first = false;
                };

                (stringify_field(fields), ...);
            }, MetaScheme.fields);

            out += Keywords::Chars::StructClose;
            return out;
        }

        static std::string toStringShort(const target_t& val)
        {
            auto out = std::string(Keywords::Chars::StructOpen);
            bool first = true;

            std::apply([&](const auto&... fields)
            {
                auto stringify_field = [&]<typename TField>(const TField& field)
                {
                    if (!first) out += ' ';
                    out += TypeParser::toStringShort<typename TField::converter_t>(field.get(val));
                    first = false;
                };

                (stringify_field(fields), ...);
            }, MetaScheme.fields);

            out += Keywords::Chars::StructClose;
            return out;
        }
    };
}
