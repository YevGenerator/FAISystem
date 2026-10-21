#pragma once
#include "field.hpp"
#include "scheme.hpp"
#include "struct_converter.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/net_bool_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgInputScheme(
        Field<DoubleConverter, &dto::KgInput::a>{Keywords::Params::InA}
        , Field<NumberConverter<std::uint64_t>, &dto::KgInput::b>{Keywords::Params::InB}
        , Field<NetBoolConverter, &dto::KgInput::c>{Keywords::Params::InC}
        , Field<DoubleConverter, &dto::KgInput::g>{Keywords::Params::InG}
        , Field<DoubleConverter, &dto::KgInput::v>{Keywords::Params::InV}
    );

    using KgInputConverter = StructConverter<KgInputScheme>;

    static_assert(IsStructConverter<KgInputConverter>);
}
