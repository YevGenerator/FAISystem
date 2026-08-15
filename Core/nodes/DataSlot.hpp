#pragma once
#include "types/basic.hpp"
#include "types/NetField.hpp"

namespace NodeSystem::Core::Nodes {
    struct InputMessage {
        types::NetDouble alpha;
        types::UsInt theta{};
    };

    struct KgInput {
        types::NetBool c;
        types::NetDouble a;
        types::UsInt b{};
        types::NetDouble v;
        types::NetDouble g;
    };

    struct KgOutput {
        types::NetBool c;
        types::NetDouble a;
    };

    struct InputDataSlot {
        KgInput kgt;
        types::NetDouble alpha;
        types::UsInt theta{};
    };
} // namespace NodeSystem::Core::Nodes
