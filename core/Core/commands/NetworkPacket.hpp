#pragma once
#include "events/EventHeader.hpp"


namespace NodeSystem::Core::Commands {
#pragma pack(push, 1)
    template<typename TContent>
    struct NetworkPacket {
        types::Byte commandId{};
        Events::EventHeader eventHeader{};
        TContent data;

        static constexpr auto headerSize() {
            return sizeof(commandId) + sizeof(eventHeader);
        }

        static constexpr auto dataSize() {
            return sizeof(TContent);
        }

        static constexpr auto size() {
            return headerSize() + dataSize();
        }
    };
#pragma pack(pop)
} // namespace NodeSystem::Core::Commands
