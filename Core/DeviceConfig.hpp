#pragma once
#include "commands/CmdTypes.hpp"
#include "types/basic.hpp"

namespace NodeSystem::Core {
    struct DeviceConfig {
        types::ID deviceId{0};
        Commands::KgIP serverAddress{};
    };
} // namespace NodeSystem::Core
