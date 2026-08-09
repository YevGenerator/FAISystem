#pragma once

#include "DataSlot.hpp"
#include "NodeId.hpp"

namespace NodeSystem::Core::Nodes {
#pragma pack(push, 1)
    struct NodeResultMessage {
        NodeId nodeId{};
        types::NetDouble alpha;
    };

    struct NodeProcessMessage {
        NodeId nodeId{};
        types::ID slotId{};
        types::NetDouble alpha;
    };

    struct NodeResultForwardMessage {
        types::ID deviceId{};
        NodeId nodeId{};
        types::NetDouble alpha;
    };

    struct NodeTimedProcessMessage {
        types::ID slotId{};
        types::NetDouble alpha;
        types::UsInt theta{};
    };

#pragma pack(pop)
} // namespace NodeSystem::Core::Nodes
