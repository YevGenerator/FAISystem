#pragma once
#include <zmq.hpp>

#include "connect_policy.hpp"
#include "../fixed_string.hpp"
#include "protocol_policy.hpp"

namespace NodeSystem::ZMCore::Queues::Policies {
    using WorkerPullConnectPolicy = ConnectPullPolicy<"inproc://workers">;
    using WorkerPushConnectPolicy = ConnectPushPolicy<"inproc://workers">;
    using RouterPullConnectPolicy = ConnectPullPolicy<"inproc://router">;
    using RouterPushConnectPolicy = ConnectPushPolicy<"inproc://router">;
    using ForwardPushConnectPolicy = ConnectPushPolicy<"inproc://forward">;
    using ForwardPullConnectPolicy = BindPullPolicy<"inproc://forward">;
    //using WorkerPullConnectPolicy = ConnectPullPolicy<"inproc://workers">;
    //using WorkerPullConnectPolicy = ConnectPullPolicy<"inproc://workers">;

    using NodeResultPullPolicy = PullerBase<Core::Commands::NodeResultPacket>;
    using NodeResultPushPolicy = PusherBase<Core::Commands::NodeResultPacket>;
    using NodeResultForwardPolicy = PusherBase<Core::Commands::NodeTransportPacket>;
    using NodeProcessPullPolicy = PullerBase<Core::Commands::NodeProcessPacket>;
    using NodeProcessPushPolicy = PusherBase<Core::Commands::NodeProcessPacket>;

    static_assert(IsPushProtocolPolicy<NodeProcessPushPolicy>);
} // namespace NodeSystem::ZMCore::Queues::Policies
