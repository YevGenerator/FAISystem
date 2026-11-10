#pragma once
#include "RouteTable.hpp"
#include "commands/CommandList.hpp"
#include "logger/Logger.hpp"
#include "queue/concepts/is_queue_pull.hpp"
#include "queue/concepts/is_queue_push.hpp"
namespace NodeSystem::Core::Nodes {
template<Queue::Concepts::IsQueueProcessPush TProcessQueuePush, Queue::Concepts::IsQueueForwardPush TForwardQueuePush, Queue::Concepts::IsQueueResultPull TResultQueuePull>
class Router {
public:
    RouteTable routes{};
    TProcessQueuePush pusher;
    TForwardQueuePush forward;
    TResultQueuePull puller;
    Router(TProcessQueuePush out, TForwardQueuePush fwd, TResultQueuePull in)
        : pusher(std::move(out)), forward(std::move(fwd)), puller(std::move(in)) {}
    void routeNewPacket() {
        auto packet = puller.pull();
        if (!packet) return;
        auto accepted = LoggerBin.newEvent(packet->eventHeader);
        LoggerBin.log(types::logs::LogRouterAccept{packet->data}, accepted);
        routes.route(packet->data,
            [&](const NodeProcessMessage& msg) {
                auto header = LoggerBin.newEvent(accepted);
                LoggerBin.log(types::logs::LogRouterSendInner{msg}, header);
                pusher.push(Commands::CommandList::CreatePacket(msg, header));
            },
            [&](const NodeResultForwardMessage& msg) {
                auto header = LoggerBin.newEvent(accepted);
                LoggerBin.log(types::logs::LogRouterSendForward{msg}, header);
                forward.push(Commands::CommandList::CreatePacket(msg, header));
            });
    }
};
}