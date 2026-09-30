#pragma once
#include "field.hpp"
#include "kg_input_converter.hpp"
#include "kg_output_converter.hpp"
#include "scheme.hpp"
#include "../primitive/numbers_converter.hpp"
#include "../primitive/node_id_converter.hpp"
#include "../primitive/algo_type_converter.hpp"
#include "../collections/list_converter.hpp"

namespace NodeSystem::strings::convert
{
    constexpr Scheme KgNodeScheme(
        Field<IdConverter, &dto::KgNode::deviceId>{Keywords::Params::Device}
        , Field<NodeIdConverter, &dto::KgNode::id>{Keywords::Params::Id}
        , Field<AlgoNameConverter, &dto::KgNode::algo>{Keywords::Params::Algo}
        , Field<ListConverter<KgInputConverter>, &dto::KgNode::inputs>{Keywords::Params::Inputs}
        , Field<KgOutputConverter, &dto::KgNode::output>{Keywords::Params::Output}
    );

    using KgNodeConverter = StructConverter<KgNodeScheme>;

    static_assert(IsStructConverter<KgNodeConverter>);
}
