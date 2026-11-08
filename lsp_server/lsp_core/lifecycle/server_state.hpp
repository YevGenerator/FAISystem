#pragma once

#include "../fixed_string.hpp"

namespace NodeSystem::lsp::lifecycle
{
    struct ServerState
    {
        constexpr static FixedString Uninitialized = "Uninitialized";
        constexpr static FixedString Initialized = "Initialized";
        constexpr static FixedString Shutdown = "Shutdown";
        constexpr static FixedString Exited = "Exited";
    };
} // namespace NodeSystem::lsp::lifecycle