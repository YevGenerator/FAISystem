#pragma once
#include "scheme.hpp"
#include "field.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/node_id_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgBindFromScheme(
        Field<IdConverter, &dto::KgBindFrom::device>{Keywords::Params::Device},
        Field<NodeIdConverter, &dto::KgBindFrom::nodeId>{Keywords::Params::Id}
    );

    using KgBindFromConverter = StructConverter<KgBindFromScheme>;

    static_assert(IsStructConverter<KgBindFromConverter>);
}
