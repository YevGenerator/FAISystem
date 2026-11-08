#pragma once
#include <optional>
#include "NodeTable.hpp"
#include "Timer.hpp"
#include "commands/CommandList.hpp"
#include "logger/Logger.hpp"
#include "queue/concepts/is_queue_pull.hpp"
#include "queue/concepts/is_queue_push.hpp"
namespace NodeSystem::Core::Nodes {
template<Queue::Concepts::IsQueueResultPush TResultQueuePush, Queue::Concepts::IsQueueProcessPull TProcessQueuePull>
class Worker {
public:
    NodeTable& nodeTable;
    TResultQueuePush pusher;
    TProcessQueuePull puller;
    types::WorkerInt workerId{};
    Worker(NodeTable& table, TResultQueuePush out, TProcessQueuePull in, types::WorkerInt id = 0)
        : nodeTable(table), pusher(std::move(out)), puller(std::move(in)), workerId(id) {}
    void process() {
        auto packet = puller.pull();
        if (!packet) return;
        const auto& message = packet->data;
        auto accepted = LoggerBin.newEvent(packet->eventHeader);
        LoggerBin.log(types::logs::LogWorkerAccept{workerId, message}, accepted);
        auto* node = nodeTable.getNode(message.nodeId);
        if (!node || message.slotId >= node->inputs.size()) {
            LoggerBin.log(types::logs::LogNodeDenied{workerId, message}, LoggerBin.newEvent(accepted));
            return;
        }
        const auto timed = timedMessage(message);
        LoggerBin.log(types::logs::LogNodeInputFilled{
            message.nodeId, message.slotId, {.alpha = timed.alpha, .theta = timed.theta}
        }, LoggerBin.newEvent(accepted));
        const auto result = node->acceptMessage(timed);
        auto processed = LoggerBin.newEvent(accepted);
        LoggerBin.log(types::logs::LogNodeProcessed{
            node->nodeId, node->algoType, types::NetDouble{result.alpha}
        }, processed);
        const NodeResultMessage response{.nodeId = node->nodeId, .alpha = result.alpha};
        auto sent = LoggerBin.newEvent(processed);
        LoggerBin.log(types::logs::LogWorkerSend{workerId, response}, sent);
        pusher.push(Commands::CommandList::CreatePacket(response, sent));
    }
    static auto timedMessage(const NodeProcessMessage& msg) -> NodeTimedProcessMessage {
        return {.slotId = msg.slotId, .alpha = msg.alpha, .theta = Timer::elapsedNow()};
    }
};
}