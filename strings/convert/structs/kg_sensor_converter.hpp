#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgSensorScheme(
        Field<IdConverter, &dto::KgSensor::device>{Keywords::Params::Device},
        Field<IdConverter, &dto::KgSensor::id>{Keywords::Params::Id}
    );

    using KgSensorConverter = StructConverter<KgSensorScheme>;

    static_assert(IsStructConverter<KgSensorConverter>);
}
