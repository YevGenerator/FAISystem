#pragma once
#include <string_view>

#include "base_dto.hpp"

namespace NodeSystem::strings::dto
{
    template <typename T>
    struct ExtractInnerType
    {
        using type = T;
    };

    template <typename Inner, const auto& Name>
    struct ExtractInnerType<BaseDTO<Inner, Name>>
    {
        using type = Inner;
    };

    template <typename T>
    using inner_type_t = ExtractInnerType<T>::type;

}
