#pragma once
#include <array>
#include <variant>
#include "../commands/CommandList.hpp"
#include "../Timer.hpp"
#include "LogRecordTypes.hpp"
#include <fstream>
#include <type_traits>
#include "events/EventCounter.hpp"

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
            Events::EventHeader header;
            types::UsInt timeStamp{};
            P data;

            static constexpr auto headerSize() {
                return sizeof(logType) + sizeof(header) + sizeof(timeStamp);
            };
        };
#pragma pack(pop)

        template<typename P>
        static LogRecord<P> CreateLogRecord(const P &data,
                                                      const Events::EventHeader &header) {
            return LogRecord<P>{
                .header = header,
                .timeStamp = Timer::elapsedNow(),
                .data = data
            };
        }

        using Variant = std::variant<std::monostate, T...>;

        template<typename P>
        static bool readLogRecord(std::istream& in, LogRecord<P>& record) {
            return static_cast<bool>(in.read(reinterpret_cast<char*>(&record), sizeof(record)));
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
