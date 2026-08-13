#pragma once
#include "delegate.hpp"
#include "DeviceConfig.hpp"
#include "WorkerPool.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore {
    template<bool IsServer>
    struct CmdProcessorContext {
        Core::DeviceConfig &deviceConfig;
        WorkerPool &workerMaster;
        std::conditional_t<IsServer, Queues::ZMBusServerTcp, Queues::ZMBusClientTcp>& tcpQueue;
        RunDelegate run;
        std::atomic<bool>& need_reconnect;
        Queues::ZMRouterPush& routerQueue;
        ZMSensors::CoreSensors& sensors;
        Core::Nodes::NodeTable& nodeTable;
        Core::Nodes::RouteTable& routeTable;
    };
} // namespace NodeSystem::ZMCore