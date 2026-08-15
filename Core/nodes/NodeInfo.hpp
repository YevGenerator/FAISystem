#pragma once
#include <span>

#include "DataSlot.hpp"
#include "NodeId.hpp"

namespace NodeSystem::Core::Nodes {
    struct NodeCreateInfo {
        NodeId id{};
        types::Byte algoType{0};
        KgOutput output;
        types::ID inputs_count{};
    };

    struct NodeAddInputInfo {
        types::ID inputId{};
        KgInput inputData;
    };

    struct NodeAlgoInfo {
        const InputMessage currentInput{};
        std::span<const InputDataSlot> inputs;
        const KgOutput &output;
        bool &state;
    };
} // namespace NodeSystem::Core::Nodes
