#pragma once
#include "types/basic.hpp"

namespace NodeSystem::Core::Events {
#pragma pack(push, 1)
    struct EventId {
        types::ID deviceId{};
        types::EventIDInt localEventId{1};

        static constexpr auto null() -> EventId {
            return {.deviceId = 0, .localEventId = 0};
        }

        [[nodiscard]]
        constexpr auto isNull() const -> bool {
            return this->localEventId == 0;
        }
    };
#pragma pack(pop)
} // namespace NodeSystem::Core::Events
