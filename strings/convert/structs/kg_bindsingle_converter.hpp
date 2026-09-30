#pragma once
#include "field.hpp"
#include "kg_bindfrom_converter.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgBindSingleScheme(
        Field<KgBindFromConverter, &dto::KgBindSingle::from>{Keywords::Params::From}
        , Field<IdConverter, &dto::KgBindSingle::deviceTo>{Keywords::Params::Id}
    );

    using KgBindSingleConverter = StructConverter<KgBindSingleScheme>;

    static_assert(IsStructConverter<KgBindSingleConverter>);
}
