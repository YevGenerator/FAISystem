#pragma once
#include <tuple>
#include <string>

namespace NodeSystem::Visual {
    enum class ArchCompType {
        None,
        Sensor,
        Router,
        Worker,
        TcpRx,
        TcpTx,
        Queue_Router,
        Queue_Worker,
        Queue_Forward
    };

    using ArchPinKey = std::tuple<int, ArchCompType, int>;

    struct ArchFlight {
        float emitTime;
        float arrivalTime;
        std::string traceName;
        ArchPinKey fromPin;
        ArchPinKey toPin;

        std::string payloadInfo;
    };
}