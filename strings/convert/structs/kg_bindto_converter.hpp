#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/node_id_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgBindToScheme(
        Field<IdConverter, &dto::KgBindTo::device>{Keywords::Params::Device}
        , Field<NodeIdConverter, &dto::KgBindTo::nodeId>{Keywords::Params::Id}
        , Field<IdConverter, &dto::KgBindTo::in_index>{Keywords::Params::In}
    );

    using KgBindToConverter = StructConverter<KgBindToScheme>;

    static_assert(IsStructConverter<KgBindToConverter>);
}
