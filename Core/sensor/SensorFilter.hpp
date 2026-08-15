#pragma once

#include <optional>
#include <cmath>

namespace NodeSystem::Core::Sensors {
    struct SensorFilter {
        types::Float baseLine = 0.0;
        types::Float epsilon = 0.2;

        [[nodiscard]]
        auto filter(types::Float rawValue) const -> std::optional<types::Float> {
            if (std::abs(rawValue - baseLine) > epsilon) {
                return rawValue;
            }
            return std::nullopt;
        }
    };
} // namespace NodeSystem::Core::Sensors
