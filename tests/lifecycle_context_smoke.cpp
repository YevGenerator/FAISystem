#include <stdexcept>
#include "Timer.hpp"
#include "ZMCore/CmdProcessor.hpp"

#define CHECK(expr) do { if (!(expr)) throw std::runtime_error("Check failed: " #expr); } while(false)

struct LifecycleProbe {
    int starts = 0;
    int waits = 0;
    bool running = false;
    int runs = 0;
    void start() { ++starts; }
    void wait() { ++waits; }
    void run(bool value) { running = value; ++runs; }
};

int main() {
    using namespace NodeSystem;
    Core::Timer::init();
    Core::DeviceConfig device{};
    device.deviceId = 2;
    LifecycleProbe probe;

    ZMCore::CmdProcessorContext<false> context{};
    context.deviceConfig = &device;
    context.startBus = ZMCore::ReconnectDelegate::create<&LifecycleProbe::start>(&probe);
    context.awaitBusReady = ZMCore::ReconnectDelegate::create<&LifecycleProbe::wait>(&probe);
    context.run = ZMCore::RunDelegate::create<&LifecycleProbe::run>(&probe);
    ZMCore::CmdProcessor<false> processor;
    processor.init(context);

    Core::Commands::CmdIp address{};
    address.port = 7779;
    processor.process(Core::Commands::CommandList::CreatePacket(address));
    CHECK(probe.starts == 1);
    CHECK(probe.waits == 1);
    CHECK(processor.ipApplied);

    bool refusedDuplicateIp = false;
    try {
        processor.process(Core::Commands::CommandList::CreatePacket(address));
    } catch (const std::runtime_error&) {
        refusedDuplicateIp = true;
    }
    CHECK(refusedDuplicateIp);

    processor.process(Core::Commands::CommandList::CreatePacket(
        Core::Commands::CmdRun{.device = 2, .toRun = true}));
    CHECK(probe.running);
    CHECK(probe.runs == 1);
    processor.process(Core::Commands::CommandList::CreatePacket(
        Core::Commands::CmdRun{.device = 2, .toRun = false}));
    CHECK(!probe.running);
    CHECK(probe.runs == 2);

    processor.process(Core::Commands::CommandList::CreatePacket(
        Core::Commands::CmdRun{.device = 3, .toRun = true}));
    CHECK(probe.runs == 2);
}
