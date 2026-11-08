#include <stdexcept>
#define CHECK(cond) do { if (!(cond)) throw std::runtime_error("Check failed: " #cond); } while(false)
#include <filesystem>
#include <fstream>
#include <iostream>
#include "logger/Logger.hpp"
#include "nodes/Node.hpp"

int main() {
    using namespace NodeSystem::Core;
    Timer::init();
    LoggerBin.setDeviceId(7);
    auto root = LoggerBin.newEvent();
    auto child = LoggerBin.newEvent(root);
    CHECK(root.parentId.isNull());
    CHECK(root.traceId.deviceId == 7);
    CHECK(root.eventId.deviceId == 7);
    CHECK(!root.eventId.isNull());
    CHECK(child.parentId.deviceId == root.eventId.deviceId);
    CHECK(child.parentId.localEventId == root.eventId.localEventId);
    CHECK(child.traceId.localEventId == root.traceId.localEventId);

    const auto path = std::filesystem::temp_directory_path() / "fai-logging-smoke.bin";
    LoggerBin.open(path);
    LoggerBin.log(types::logs::LogSetDeviceId{7}, root);
    LoggerBin.log(types::logs::LogSetWorkers{2}, child);
    std::ifstream stream(path, std::ios::binary);
    LogRecordList::LogRecord<types::logs::LogSetDeviceId> first{};
    LogRecordList::LogRecord<types::logs::LogSetWorkers> second{};
    CHECK(LogRecordList::readLogRecord(stream, first));
    CHECK(LogRecordList::readLogRecord(stream, second));
    CHECK(first.logType == LogRecordList::id<types::logs::LogSetDeviceId>);
    CHECK(second.logType == LogRecordList::id<types::logs::LogSetWorkers>);
    CHECK(first.data.deviceId == 7);
    CHECK(second.data.workerAmount == 2);
    CHECK(second.header.parentId.localEventId == first.header.eventId.localEventId);
    CHECK(stream.peek() == EOF);

    Nodes::Node node(Nodes::NodeCreateInfo{
        .id = {.level = 1, .index = 5},
        .algoType = 0,
        .output = {.c = true, .a = 0.5},
        .inputs_count = 2
    });
    CHECK(node.inputs.size() == 2);
    node.addInput({.inputId = 1, .inputData = {
        .c = true, .a = 0.5, .b = 0, .v = 0.1, .g = 0.9
    }});
    CHECK(node.inputs[1].kgt.g.get() == 0.9);
    std::cout << "logging smoke test passed\n";
    return 0;
}
