#pragma once
#include "EventId.hpp"

namespace NodeSystem::Core::Events {
#pragma pack(push, 1)
    struct EventHeader {
        EventId eventId;
        EventId parentId;
        EventId traceId;
    };
#pragma pack(pop)

} // namespace NodeSystem::Core::Events
