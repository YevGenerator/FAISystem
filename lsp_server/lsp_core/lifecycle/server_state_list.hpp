#pragma once

#include <cstdint>

#include "server_state.hpp"
#include "../fixed_string.hpp"

namespace NodeSystem::lsp::lifecycle
{
    template <FixedString... states>
    struct ServerStateListBase
    {
        static constexpr auto count = sizeof ...(states);

        template <FixedString str>
        static constexpr std::uint64_t id = []
        {
            constexpr auto index = []
            {
                std::uint64_t i = 0;
                auto found = ((str == states ? true : (++i, false)) || ...);
                return i;
            }();
            static_assert(index != count, "Unknown server state");
            return index;
        }();
    };

    using ServerStateList = ServerStateListBase<
        ServerState::Uninitialized
        , ServerState::Initialized
        , ServerState::Shutdown
        , ServerState::Exited
    >;
} // namespace NodeSystem::lsp::lifecycle
