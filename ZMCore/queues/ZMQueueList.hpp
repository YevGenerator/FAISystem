#pragma once
#include "ZMQueueBase.hpp"
#include "policies/policy_list.hpp"

namespace NodeSystem::ZMCore::Queues {
    using ZMWorkerPull = ZMQueueBase<
        Policies::WorkerPullConnectPolicy,
        Policies::NodeProcessPullPolicy
    >;

    using ZMWorkerPush = ZMQueueBase<
        Policies::WorkerPushConnectPolicy,
        Policies::NodeProcessPushPolicy
    >;

    using ZMRouterPull = ZMQueueBase<
        Policies::RouterPullConnectPolicy,
        Policies::NodeResultPullPolicy
    >;

    using ZMRouterPush = ZMQueueBase<
        Policies::RouterPushConnectPolicy,
        Policies::NodeResultPushPolicy
    >;

    using ZMForwardPush = ZMQueueBase<
        Policies::ForwardPushConnectPolicy,
        Policies::NodeResultForwardPolicy
    >;

    using ZMBusServerTcp = ZMQueueBase<Policies::ServerTcpConnectPolicy, Policies::ServerTcpProtocolPolicy>;
    using ZMBusClientTcp = ZMQueueBase<Policies::ClientTcpConnectPolicy, Policies::ClientTcpProtocolPolicy>;

    using ZMBusServerForward = ZMQueueBase<Policies::ForwardPullConnectPolicy, Policies::ServerTcpProtocolPolicy>;
    using ZMBusClientForward = ZMQueueBase<Policies::ForwardPullConnectPolicy, Policies::ClientTcpProtocolPolicy>;

    using ZMBusServerForwardPush = ZMQueueBase<Policies::ForwardPushConnectPolicy, Policies::ServerTcpProtocolPolicy>;
    using ZMBusClientForwardPush = ZMQueueBase<Policies::ForwardPushConnectPolicy, Policies::ClientTcpProtocolPolicy>;

    static_assert(Core::Queue::Concepts::IsQueueProcessPull<ZMWorkerPull>);
    static_assert(Core::Queue::Concepts::IsQueueProcessPush<ZMWorkerPush>);
    static_assert(Core::Queue::Concepts::IsQueueResultPull<ZMRouterPull>);
    static_assert(Core::Queue::Concepts::IsQueueForwardPush<ZMForwardPush>);
} // namespace NodeSystem::ZMCore::Queues
