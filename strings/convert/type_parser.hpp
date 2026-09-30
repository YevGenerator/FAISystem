#pragma once
#include <optional>

#include "primitive/is_string_converter.hpp"
#include "structs/is_struct_converter.hpp"
#include "../preparser.hpp"
#include "../ParserContext.hpp"

namespace NodeSystem::strings::convert
{
    template <typename Target>
    concept IsAnyStringConverter = IsStringConverter<Target> || IsStructConverter<Target>;

    class TypeParser
    {
    public:
        template <IsAnyStringConverter TConverter>
        static std::optional<typename TConverter::target_t> parseToken(
            std::string_view& stream, const ParserContext& ctx)
        {
            std::string_view token = Preparser::consumeNextValue(stream);
            if (token.empty()) return std::nullopt;
            return resolveOrParse<TConverter>(token, ctx);
        }

        template <IsAnyStringConverter TConverter>
        static std::optional<typename TConverter::target_t> resolveOrParse(
            std::string_view token, const ParserContext& ctx)
        {
            using target_t = TConverter::target_t;
            if (auto val = ctx.resolve<target_t>(token))
                return val;

            if constexpr (IsStructConverter<TConverter>)
                return TConverter::fromString(token, ctx);
            else
                return TConverter::fromString(token);
        }

        template <IsAnyStringConverter TConverter>
        static auto toString(const TConverter::target_t& val) -> std::string
        {
            return TConverter::toString(val);
        }

        template <IsAnyStringConverter TConverter>
        static auto toStringShort(const TConverter::target_t& val) -> std::string
        {
            if constexpr (IsStructConverter<TConverter>)
                return TConverter::toStringShort(val);
            else
                return TConverter::toString(val);
        }
    };
}
