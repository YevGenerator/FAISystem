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
        Queue_Router,   // Додано: Черга на вхід до роутера
        Queue_Worker,   // Додано: Черга до пулу воркерів
        Queue_Forward   // Додано: Черга на відправку в мережу
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