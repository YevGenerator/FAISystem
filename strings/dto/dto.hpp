#pragma once
#include <vector>

#include "base_dto.hpp"
#include "types/basic.hpp"
#include "types/NetField.hpp"
#include "nodes/NodeId.hpp"
#include "nodes/DataSlot.hpp"
#include "commands/CmdTypes.hpp"

namespace NodeSystem::strings::dto
{
    using Id = Core::types::ID;
    using NodeId = Core::Nodes::NodeId;
    using Bool = Core::types::NetBool;
    using Float = Core::types::Float;
    using AlgoType = Core::types::Byte;

    using KgIp = Core::Commands::KgIP;
    using KgRun = Core::Commands::CmdRun;
    using KgSensor = Core::Commands::CmdSensorCreate;
    using KgOutput = Core::Nodes::KgOutput;
    using KgInput = Core::Nodes::KgInput;

    struct KgBindFrom
    {
        Id device;
        NodeId nodeId;
    };

    struct KgBindTo
    {
        Id device;
        NodeId nodeId;
        Id in_index;
    };

    struct KgBindSingle
    {
        KgBindFrom from;
        Id deviceTo;
    };

    struct KgNode
    {
        Id deviceId{};
        NodeId id{};
        AlgoType algo{};
        KgOutput output{};
        std::vector<KgInput> inputs;
    };

    struct KgBindDouble
    {
        KgBindFrom from{};
        std::vector<KgBindTo> to;
    };
}
