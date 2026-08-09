#pragma once

#include "types/ConfigTypes.hpp"
#include "types.hpp"

namespace NodeSystem::Core::types::logs {
#pragma pack(push, 1)
    struct LogSetDeviceId {
        ID deviceId;
    };

    struct LogSetWorkers {
        SmallInt workerAmount;
    };

    struct LogSetIp {
        KgIP ipData;
    };

    struct LogRun {
        ID device{};
        NetBool toRun;
        NetBool isSender;
    };

    struct LogCreateSensor {
        ID device{};
        ID id_val{};
        NetBool isSender;
        NetBool isRandom;
        MsInt period{};
        NetDouble toEmit;
    };


    struct LogCreateNode {
        ID device{};
        ID id_lvl{};
        ID id_val{};
        Byte algo{0};
        KgOutput output;
        ID inputs_count{};
        NetBool isSender;
    };

    struct LogNodeAddInput {
        ID device{};
        ID id_lvl{};
        ID id_val{};
        ID input_index{};
        KgInput input_data;
        NetBool isSender;
    };

    struct LogDoubleBind {
        KgBindFrom from{};
        KgBindTo to{};
        NetBool isSender;
    };

    struct LogSingleBind {
        KgBindFrom from{};
        ID deviceTo{};
        NetBool isSender;
    };

    struct LogSensorOutEvent {
        NodeResultMessage result;
    };

    struct LogRouterAccept {
        NodeResultMessage result;
    };

    struct LogRouterSendInner {
        NodeProcessMessage message;
    };

    struct LogRouterSendForward {
        NodeResultMessage result;
    };

    struct LogWorkerAccept {
        SmallInt workerId{};
        NodeProcessMessage message;
    };

    struct LogNodeInputFilled {
        NodeId nodeId{};
        ID slotId{};
        ProcessData data;
    };

    struct LogNodeProcessed {
        NodeId nodeId{};
        Byte algo{};
        NetDouble result;
    };
    struct LogNodeDenied {
        SmallInt workerId{};
        NodeProcessMessage message;
    };

    struct LogWorkerSend {
        SmallInt workerId{};
        NodeResultMessage result;
    };

    struct LogReceivedTcpCommand {
        ID deviceFrom;
        ID deviceTo;
    };

    struct LogSendTcpCommand {
        ID deviceFrom;
        ID deviceTo;
    };

#pragma pack(pop)
}
