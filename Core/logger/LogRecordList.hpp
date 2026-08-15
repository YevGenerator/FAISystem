#pragma once
#include <array>
#include <variant>
#include "../commands/CommandList.hpp"
#include "../Timer.hpp"
#include "LogRecordTypes.hpp"
#include <fstream>

namespace NodeSystem::Core {
    template<typename... T>
    struct LogRecordRegistry {
        static constexpr types::Byte count = sizeof ...(T);

        template<typename K>
        static constexpr types::Byte id = [] {
            types::Byte index = 0;
            auto found = ((std::is_same_v<K, T> ? true : (++index, false)) || ...);
            return index;
        }();

#pragma pack(push, 1)

        template<typename P>
        struct LogRecord {
            types::Byte logType = id<P>;
            EventCounter::EventHeader header;
            types::UsInt timeStamp{};
            P data;

            static constexpr auto headerSize() {
                return sizeof(logType) + sizeof(header) + sizeof(timeStamp);
            };
        };
#pragma pack(pop)

        template<typename P>
        static constexpr LogRecord<P> CreateLogRecord(const P &data,
                                                      const EventCounter::EventHeader &header = EventCounter::event()) {
            return LogRecord<P>{
                .header = header,
                .timeStamp = Timer::elapsedNow(),
                .data = data
            };
        }

        using Variant = std::variant<std::monostate, T...>;

        template<typename P>
        static constexpr auto readLogRecord(std::ifstream& in) {
            LogRecord<Variant> record;
            in.read( reinterpret_cast<char *>(&record.header), sizeof(record.header));
            in.read(reinterpret_cast<char *>(&record.data), sizeof(P));
            return record;
        }
    };

    using namespace types;
    using LogRecordList = LogRecordRegistry<
        logs::LogSetDeviceId,
        logs::LogSetWorkers,
        logs::LogSetIp,
        logs::LogRun,
        logs::LogCreateSensor,
        logs::LogCreateNode,
        logs::LogNodeAddInput,
        logs::LogDoubleBind,
        logs::LogSingleBind,
        logs::LogSensorOutEvent,
        logs::LogRouterAccept,
        logs::LogRouterSendInner,
        logs::LogRouterSendForward,
        logs::LogWorkerAccept,
        logs::LogNodeInputFilled,
        logs::LogNodeProcessed,
        logs::LogNodeDenied,
        logs::LogWorkerSend,
        logs::LogReceivedTcpCommand,
        logs::LogSendTcpCommand
    >;
}
