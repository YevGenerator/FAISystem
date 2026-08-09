#pragma once
#include "SensorFilter.hpp"
#include <optional>

#include "nodes/NodeMessage.hpp"

namespace NodeSystem::Core::Sensors {
    struct Sensor {
        types::ID id{};
        types::ID pinIndex{};
        SensorFilter filter{};

        Sensor() = default;

        Sensor(const types::ID id, const types::ID pinIndex, SensorFilter filter = {}) : id(id), pinIndex(pinIndex),
            filter(filter) {
        }

        [[nodiscard]]
        auto asMessage(const types::Float rawValue) const -> std::optional<Nodes::NodeResultMessage> {
            if (const auto filtered = filter.filter(rawValue)) {
                return Nodes::NodeResultMessage{
                    .nodeId = nodeId(),
                    .alpha = *filtered,
                };
            }
            return std::nullopt;
        }

        [[nodiscard]]
        auto nodeId() const -> Nodes::NodeId {
            return {.level = 0, .index = id};
        }
    };
} // namespace NodeSystem::Core::Sensors
