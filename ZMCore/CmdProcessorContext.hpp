#pragma once
#include "delegate.hpp"
#include "DeviceConfig.hpp"
#include "WorkerPool.hpp"
#include "ZMSensors.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore
{
    template <bool IsServer>
    struct CmdProcessorContext
    {
        Core::DeviceConfig* deviceConfig = nullptr;
        WorkerPool* workerMaster = nullptr;
        std::conditional_t<IsServer, Queues::ZMBusServerTcp, Queues::ZMBusClientTcp>* tcpQueue = nullptr;
        RunDelegate run{};
        std::atomic<bool>* need_reconnect = nullptr;
        Queues::ZMRouterPush* routerQueue = nullptr;
        ZMSensors::CoreSensors* sensors = nullptr;
        Core::Nodes::NodeTable* nodeTable = nullptr;
        Core::Nodes::RouteTable* routeTable = nullptr;
    };
} // namespace NodeSystem::ZMCore
