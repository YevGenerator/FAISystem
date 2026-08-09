#pragma once
#include "NodeTable.hpp"
#include "RouteTable.hpp"
#include "Timer.hpp"
#include "commands/CommandList.hpp"
#include "queue/concepts/is_queue_pull.hpp"
#include "queue/concepts/is_queue_push.hpp"

namespace NodeSystem::Core::Nodes {
    template<
        Queue::Concepts::IsQueueResultPush TResultQueuePush,
        Queue::Concepts::IsQueueProcessPull TProcessQueuePull
    >
    class Worker {
    public:
        NodeTable &nodeTable;
        TResultQueuePush pusher;
        TProcessQueuePull puller;

        Worker(NodeTable &nodeTable, TResultQueuePush pusher, TProcessQueuePull puller)
            : nodeTable(nodeTable), pusher(std::move(pusher)), puller(std::move(puller)) {
        }

        void process() {
            std::optional<Commands::NodeProcessPacket> packet = puller.pull();
            if (packet.has_value()) {
                const auto &message = packet.value().data;
                auto *node = nodeTable.getNode(message.nodeId);
                auto result = node->acceptMessage(timedMessage(message));
                auto newPacket = Commands::CommandList::CreatePacket(NodeResultMessage{
                    .nodeId = node->nodeId, .alpha = result.alpha,
                });
                pusher.push(newPacket);
            }
        }

        static auto timedMessage(const NodeProcessMessage &msg) -> NodeTimedProcessMessage {
            return {
                .slotId = msg.slotId,
                .alpha = msg.alpha,
                .theta = Timer::elapsedNow(),
            };
        }
    };
} // namespace NodeSystem::Core::Nodes
