#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/net_bool_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgRunScheme(
        Field<IdConverter, &dto::KgRun::device>{Keywords::Params::Device},
        Field<NetBoolConverter, &dto::KgRun::toRun>{Keywords::Params::ToRun}
    );

    using KgRunConverter = StructConverter<KgRunScheme>;

    static_assert(IsStructConverter<KgRunConverter>);
}
