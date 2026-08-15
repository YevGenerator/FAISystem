#pragma once

#include <shared_mutex>
#include <unordered_map>

#include "Node.hpp"

namespace NodeSystem::Core::Nodes {
    class NodeTable {
    public:
        std::unordered_map<NodeId, Node> nodes;

        NodeTable() = default;

        auto getNode(const NodeId id) -> Node * {
            const auto it = this->nodes.find(id);
            if (it != nodes.end()) {
                return &it->second;
            }
            return nullptr;
        }


        void createAndAddNode(const NodeCreateInfo &nodeInfo) {
            nodes.emplace(nodeInfo.id, nodeInfo);
        }


        void addInputToNode(const NodeId &id, const KgInput &kgInput, const types::ID inputId) {
            auto *node = getNode(id);
            if (node != nullptr) {
                node->addInput({.inputId = inputId, .inputData = kgInput});
            }
        }

    private:
        mutable std::shared_mutex nodes_mutex{};
    };
} // namespace NodeSystem::Core::Nodes
