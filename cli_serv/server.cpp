#include <iostream>
#include <zmq.hpp>
#include "Timer.hpp"
#include "ZMCore/WorkerPool.hpp"
#include "ZMCore/ZMRouter.hpp"
#include "ZMCore/ZMSensors.hpp"
#include "ZMCore/ZMBus.hpp"
#include "ZMCore/ThreadRunner.hpp"
#include "binary_read_config.hpp"
#include "cli_parser.hpp"

using namespace NodeSystem;

int main(int argc, char* argv[]) {
    auto options = Serv::CliParser::parse(argc, argv, true);
    Core::Timer::init();

    zmq::context_t zmqContext{1};

    ZMCore::WorkerPool workerPool{zmqContext};
    ZMCore::ZMRouter router{zmqContext};
    ZMCore::ZMSensors sensors{zmqContext};
    ZMCore::ZMBus<true> bus{zmqContext};

    if (options.deviceIdOverride) {
        bus.config.deviceId = *options.deviceIdOverride;
    }

    ZMCore::CmdProcessorContext<true> cmdContext{
        .workerMaster = &workerPool,
        .run = ZMCore::RunDelegate::create<&ZMCore::WorkerPool::run>(&workerPool),
        .sensors = &sensors.sensors,
        .nodeTable = &workerPool.nodeStore,
        .routeTable = &router.coreRouter.routes
    };

    bus.init(cmdContext);

    std::cout << "[Server] Loading config from " << options.configPath << "...\n";
    if (!Serv::BinaryConfigReader::loadAndExecute(options.configPath, bus.processor)) {
        std::cerr << "[Server] Failed to load config!\n";
        return 1;
    }

    ZMCore::ThreadRunner<ZMCore::ZMRouter&> routerThread(router);
    ZMCore::ThreadRunner<ZMCore::ZMBus<true>&> busThread(bus);
    ZMCore::ThreadRunner<ZMCore::ZMSensors&> sensorsThread(sensors);

    routerThread.launch();
    busThread.launch();
    sensorsThread.launch();

    std::cout << "[Server] Running. Press Enter to shutdown...\n";
    std::cin.get();

    return 0;
}