#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/net_bool_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgOutputScheme(
        Field<DoubleConverter, &dto::KgOutput::a>{Keywords::Params::OutA},
        Field<NetBoolConverter, &dto::KgOutput::c>{Keywords::Params::OutC}
    );

    using KgOutputConverter = StructConverter<KgOutputScheme>;

    static_assert(IsStructConverter<KgOutputConverter>);
}
