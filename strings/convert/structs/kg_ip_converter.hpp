#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/ipv4_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgIpScheme(
        Field<Ipv4Converter, &dto::KgIp::host>{Keywords::Params::Host},
        Field<NumberConverter<std::uint16_t>, &dto::KgIp::port>{Keywords::Params::Port}
    );

    using KgIpConverter = StructConverter<KgIpScheme>;

    static_assert(IsStructConverter<KgIpConverter>);
}
