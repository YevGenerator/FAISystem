#include <optional>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "Timer.hpp"
#include "events/EventCounter.hpp"
#include "logger/Logger.hpp"
#include "nodes/Router.hpp"
#include "nodes/Worker.hpp"

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error("Check failed: " #condition); } while (false)

using namespace NodeSystem::Core;
struct ResultPull {
    using target_t = Commands::NodeResultPacket;
    std::optional<target_t> current;
    auto pull() -> std::optional<target_t> {
        auto packet = std::move(current);
        current.reset();
        return packet;
    }
};
struct ProcessPull {
    using target_t = Commands::NodeProcessPacket;
    std::optional<target_t> current;
    auto pull() -> std::optional<target_t> {
        auto packet = std::move(current);
        current.reset();
        return packet;
    }
};
struct ProcessPush {
    std::vector<Commands::NodeProcessPacket> records;
    void push(const Commands::NodeProcessPacket& packet) { records.push_back(packet); }
};
struct ForwardPush {
    std::vector<Commands::NodeTransportPacket> records;
    void push(const Commands::NodeTransportPacket& packet) { records.push_back(packet); }
};
struct ResultPush {
    std::vector<Commands::NodeResultPacket> records;
    void push(const Commands::NodeResultPacket& packet) { records.push_back(packet); }
};

int main() {
    Timer::init();
    LoggerBin.setDeviceId(1);
    auto root = LoggerBin.newEvent();
    Nodes::NodeResultMessage origin{.nodeId = {0, 10}, .alpha = 0.75};
    auto packet = Commands::CommandList::CreatePacket(origin, root);

    Nodes::Router<ProcessPush, ForwardPush, ResultPull> router{
        ProcessPush{}, ForwardPush{}, ResultPull{packet}
    };
    router.routes.add_address({0, 10}, Nodes::InternalAddress{
        .targetNodeId = {1, 5}, .slotId = 0
    });
    router.routes.add_address({0, 10}, Nodes::ExternalAddress{2});
    router.routeNewPacket();
    CHECK(router.pusher.records.size() == 1);
    CHECK(router.forward.records.size() == 1);
    const auto routed = router.pusher.records.front();
    CHECK(routed.data.nodeId.level == 1);
    CHECK(routed.data.nodeId.index == 5);
    CHECK(routed.data.slotId == 0);
    CHECK(routed.eventHeader.traceId.localEventId == root.traceId.localEventId);
    CHECK(!routed.eventHeader.parentId.isNull());
    CHECK(router.forward.records.front().data.deviceId == 2);

    Nodes::NodeTable table;
    table.createAndAddNode(Nodes::NodeCreateInfo{
        .id = {1, 5}, .algoType = 0,
        .output = {.c = true, .a = 0.7}, .inputs_count = 1
    });
    table.addInputToNode({1, 5}, Nodes::KgInput{
        .c = true, .a = 0.5, .b = 0, .v = 0.1, .g = 0.5
    }, 0);

    Nodes::Worker<ResultPush, ProcessPull> worker{
        table, ResultPush{}, ProcessPull{routed}, 0
    };
    worker.process();
    CHECK(worker.pusher.records.size() == 1);
    const auto& response = worker.pusher.records.front();
    CHECK(response.data.nodeId.level == 1);
    CHECK(response.data.nodeId.index == 5);
    CHECK(response.eventHeader.traceId.localEventId == root.traceId.localEventId);
    CHECK(response.eventHeader.traceId.deviceId == root.traceId.deviceId);
    CHECK(!response.eventHeader.parentId.isNull());

    Nodes::Worker<ResultPush, ProcessPull> invalid{
        table, ResultPush{}, ProcessPull{
            Commands::CommandList::CreatePacket(Nodes::NodeProcessMessage{
                .nodeId = {100, 42}, .slotId = 0, .alpha = 0.5
            }, root)
        }, 0
    };
    invalid.process();
    CHECK(invalid.pusher.records.empty());
    std::cout << "core pipeline causality smoke test passed\n";
}
