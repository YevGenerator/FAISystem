#pragma once
#include "NodeTable.hpp"
#include "RouteTable.hpp"
#include "commands/CommandList.hpp"
#include "queue/concepts/is_queue_pull.hpp"
#include "queue/concepts/is_queue_push.hpp"

namespace NodeSystem::Core::Nodes {
    template<
        Queue::Concepts::IsQueueProcessPush TProcessQueuePush,
        Queue::Concepts::IsQueueForwardPush TForwardQueuePush,
        Queue::Concepts::IsQueueResultPull TResultQueuePull
    >
    class Router {
    public:
        RouteTable routes{};
        TProcessQueuePush pusher;
        TForwardQueuePush forward;
        TResultQueuePull puller;

        Router(TProcessQueuePush pusher, TForwardQueuePush forward, TResultQueuePull puller)
            : pusher(std::move(pusher)), forward(std::move(forward)), puller(std::move(puller)) {
        }

        void routeNewPacket() {
            std::optional<Commands::NodeResultPacket> possiblePacket = puller.pull();
            if (possiblePacket.has_value()) {
                auto packet = possiblePacket.value();

                this->routes.route(packet.data,
                                   [&](const NodeProcessMessage &msg) -> auto {
                                       auto newPacket = Commands::CommandList::CreatePacket(msg);
                                       pusher.push(newPacket);
                                   }, [&](const NodeResultForwardMessage &msg) -> auto {
                                       auto newPacket = Commands::CommandList::CreatePacket(msg);
                                       forward.push(newPacket);
                                   });
            }
        }
    };
} // namespace NodeSystem::Core::Nodes
