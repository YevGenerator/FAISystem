#pragma once
#include <atomic>
#include <optional>

#include "EventHeader.hpp"

namespace NodeSystem::Core::Events {
    struct EventCounter {
    private:
        inline static std::atomic<types::EventIDInt> eventCounter{1};
        inline static std::atomic<types::EventIDInt> traceCounter{1};

        static auto nextId() {
            return eventCounter.fetch_add(1, std::memory_order_relaxed);
        }

        static auto nextTraceId(const types::ID deviceId) -> EventId {
            return {
                .deviceId = deviceId,
                .localEventId = traceCounter.fetch_add(1, std::memory_order_relaxed),
            };
        }

    public:
        static auto newEvent(const types::ID deviceId,
                             const std::optional<EventHeader> &parent = std::nullopt) -> EventHeader {
            const EventId thisEventId = {.deviceId = deviceId, .localEventId = nextId()};
            EventHeader header{.eventId = thisEventId};
            if (parent) {
                header.parentId = parent->eventId;
                header.traceId = parent->traceId;
            } else {
                header.parentId = EventId::null();
                header.traceId = nextTraceId(deviceId);
            }

            return header;
        }
    };
} // namespace NodeSystem::Core::Events
