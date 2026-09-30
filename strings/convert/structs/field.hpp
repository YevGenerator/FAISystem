#pragma once
#include "../type_parser.hpp"

namespace NodeSystem::strings::convert
{
    template <IsAnyStringConverter TConverter, auto MemberPtr>
    struct Field
    {
        using converter_t = TConverter;
        using pointer_t = decltype(MemberPtr);
        std::string_view name;

        static constexpr void set(auto& obj, const TConverter::target_t& val)
        {
            obj.*MemberPtr = val;
        }

        static constexpr auto get(auto& obj)
        {
            return obj.*MemberPtr;
        }
    };
}
