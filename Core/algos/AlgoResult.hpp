#pragma once
#include "types.hpp"

namespace NodeSystem::Core::Algo {
    struct AlgoResult {
        bool toSwitch{false};
        AlgoFloat alpha{};
    };

} // namespace NodeSystem::Core::Algo