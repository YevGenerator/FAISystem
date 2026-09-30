#pragma once
#include "field.hpp"
#include "kg_bindfrom_converter.hpp"
#include "kg_bindto_converter.hpp"
#include "scheme.hpp"
#include "../collections/list_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgBindDoubleScheme(
        Field<KgBindFromConverter, &dto::KgBindDouble::from>{Keywords::Params::From}
        , Field<ListConverter<KgBindToConverter>, &dto::KgBindDouble::to>{Keywords::Params::To}
    );

    using KgBindDoubleConverter = StructConverter<KgBindDoubleScheme>;

    static_assert(IsStructConverter<KgBindDoubleConverter>);
}
