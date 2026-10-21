#pragma once

#include "DataSlot.hpp"
#include "NodeId.hpp"

#include "../algos/algoholic.hpp"
#include "NodeInfo.hpp"
#include "NodeMessage.hpp"


namespace NodeSystem::Core::Nodes {
    class Node {
    public:
        Node() = default;

        Node(const NodeCreateInfo &createInfo) : nodeId{createInfo.id}, output{createInfo.output},
                                                 algoType{createInfo.algoType} {
            this->reserveInputs(createInfo.inputs_count);
        }

        void addInput(const NodeAddInputInfo &addInputInfo) {
            this->inputs[addInputInfo.inputId].kgt = addInputInfo.inputData;
        }

        void reserveInputs(const types::ID count) {
            this->inputs.reserve(count);
        }

        auto acceptMessage(const NodeTimedProcessMessage &message) {
            auto &input = this->inputs[message.slotId];
            input.alpha = message.alpha;
            input.theta = message.theta;
            auto result = Algo::Algoholic::execute(this->algoType, this->nodeAlgoState(message));
            return result;
        }

        auto nodeAlgoState(const NodeTimedProcessMessage &message) -> NodeAlgoInfo {
            return NodeAlgoInfo{
                .currentInput = {.alpha = message.alpha, .theta = message.theta},
                .inputs = this->inputs,
                .output = this->output,
                .state = this->isUp,
            };
        }

        NodeId nodeId{};
        std::vector<InputDataSlot> inputs;
        KgOutput output;
        types::Byte algoType = 0;
        bool isUp{};

    protected:
        //mutable std::mutex mutex;
    };
} // namespace NodeSystem::Core::Nodes
