#pragma once
#include "types/basic.hpp"

namespace NodeSystem::Core::Sensors {
    struct PinDriver {
        static auto readPin(types::ID pinIndex) -> double {
            return 0.9;
        }
    };
} // namespace NodeSystem::Core::Sensors
