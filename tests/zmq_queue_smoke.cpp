#include <iostream>
#include <stdexcept>
#include <zmq.hpp>
#include "Timer.hpp"
#include "events/EventCounter.hpp"
#include "commands/CommandList.hpp"
#include "ZMCore/queues/ZMQueueList.hpp"

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error("Check failed: " #condition); } while(false)

int main() {
    using namespace NodeSystem;
    zmq::context_t context{1};
    Core::Timer::init();

    ZMCore::Queues::ZMRouterPull routerReceiver{context};
    ZMCore::Queues::ZMRouterPush routerSender{context};
    routerReceiver.init();
    routerSender.init();
    routerSender.socket.set(zmq::sockopt::sndtimeo, 2000);
    auto root = Core::Events::EventCounter::newEvent(7);
    const auto result = Core::Commands::CommandList::CreatePacket(
        Core::Nodes::NodeResultMessage{.nodeId = {0, 10}, .alpha = 0.9}, root
    );
    routerSender.push(result);
    auto delivered = routerReceiver.pull();
    CHECK(delivered.has_value());
    CHECK(delivered->data.nodeId.index == 10);
    CHECK(delivered->eventHeader.traceId.localEventId == root.traceId.localEventId);

    ZMCore::Queues::ZMWorkerPush workerSender{context};
    ZMCore::Queues::ZMWorkerPull workerReceiver{context};
    workerSender.init();
    workerReceiver.init();
    workerSender.socket.set(zmq::sockopt::sndtimeo, 2000);
    const auto process = Core::Commands::CommandList::CreatePacket(
        Core::Nodes::NodeProcessMessage{.nodeId = {1, 5}, .slotId = 0, .alpha = 0.7}, root
    );
    workerSender.push(process);
    auto workerMsg = workerReceiver.pull();
    CHECK(workerMsg.has_value());
    CHECK(workerMsg->data.nodeId.index == 5);
    CHECK(workerMsg->data.slotId == 0);

    ZMCore::Queues::ZMBusServerForward forwardReceiver{context};
    ZMCore::Queues::ZMForwardPush forwardSender{context};
    forwardReceiver.init();
    forwardSender.init();
    forwardSender.socket.set(zmq::sockopt::sndtimeo, 2000);
    const auto transport = Core::Commands::CommandList::CreatePacket(
        Core::Nodes::NodeResultForwardMessage{.deviceId = 2, .nodeId = {1, 5}, .alpha = 0.8}, root
    );
    forwardSender.push(transport);
    auto transit = forwardReceiver.pull();
    CHECK(transit.has_value());
    CHECK(transit->data.deviceId == 2);
    CHECK(transit->eventHeader.eventId.localEventId == root.eventId.localEventId);
    std::cout << "inproc ZeroMQ queues smoke test passed\n";
}
