#pragma once
#include <functional>
#include <string>
#include <types/basic.hpp>

namespace NodeSystem::Core::Nodes {
    struct NodeId {
        types::ID level;
        types::ID index;

        constexpr auto operator==(const NodeId &other) const -> bool {
            return other.level == this->level && other.index == this->index;
        }
    };
} // namespace NodeSystem::Core::Nodes

template<>
struct std::hash<NodeSystem::Core::Nodes::NodeId> {
    auto operator()(const NodeSystem::Core::Nodes::NodeId &id) const noexcept {
        const auto h1 = std::hash<NodeSystem::Core::types::ID>{}(id.level);
        const auto h2 = std::hash<NodeSystem::Core::types::ID>{}(id.index);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};
