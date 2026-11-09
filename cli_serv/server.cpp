#include <iostream>
#include <filesystem>
#include <string>
#include "logger/Logger.hpp"
#include <zmq.hpp>
#include "Timer.hpp"
#include "ZMCore/WorkerPool.hpp"
#include "ZMCore/ZMRouter.hpp"
#include "ZMCore/ZMSensors.hpp"
#include "ZMCore/ZMBus.hpp"
#include "ZMCore/ThreadRunner.hpp"
#include "ZMCore/RuntimeController.hpp"
#include "binary_read_config.hpp"
#include "cli_parser.hpp"

using namespace NodeSystem;

int main(int argc, char* argv[]) {
    auto options = Serv::CliParser::parse(argc, argv, true);
    Core::Timer::init();
    std::filesystem::path logPath = options.logPath.value_or(std::filesystem::path("server.log.bin"));
    if (!options.logPath) {
        for (unsigned i = 1; std::filesystem::exists(logPath); ++i)
            logPath = std::filesystem::path("server-" + std::to_string(i) + ".log.bin");
    }
    try { Core::LoggerBin.open(logPath); }
    catch (const std::exception& e) {
        std::cerr << "[Log] " << e.what() << "\n";
        return 1;
    }
    if (options.deviceIdOverride) Core::LoggerBin.setDeviceId(*options.deviceIdOverride);

    zmq::context_t zmqContext{1};

    ZMCore::WorkerPool workerPool{zmqContext};
    ZMCore::ZMRouter router{zmqContext};
    ZMCore::ZMSensors sensors{zmqContext};
    ZMCore::ZMBus<true> bus{zmqContext};

    if (options.deviceIdOverride) {
        bus.config.deviceId = *options.deviceIdOverride;
    }

    ZMCore::ThreadRunner<ZMCore::ZMBus<true>&> busThread(bus);
    ZMCore::RuntimeController runtime(workerPool, router, sensors);

    ZMCore::CmdProcessorContext<true> cmdContext{
        .deviceOverride = options.deviceIdOverride,
        .workerMaster = &workerPool,
        .run = ZMCore::RunDelegate::create<&ZMCore::RuntimeController::run>(&runtime),
        .startBus = ZMCore::ReconnectDelegate::create<&ZMCore::ThreadRunner<ZMCore::ZMBus<true>&>::launch>(&busThread),
        .awaitBusReady = ZMCore::ReconnectDelegate::create<&ZMCore::ZMBus<true>::awaitReady>(&bus),
        .sensors = &sensors.sensors,
        .nodeTable = &workerPool.nodeStore,
        .routeTable = &router.coreRouter.routes
    };

    bus.init(cmdContext);

    std::cout << "[Server] Loading config from " << options.configPath << "...\n";
    try {
        if (!Serv::BinaryConfigReader::loadAndExecute(options.configPath, bus.processor)) {
            std::cerr << "[Server] Failed to load config!\n";
            busThread.shutdown();
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "[Server] Configuration/runtime error: " << error.what() << '\n';
        busThread.shutdown();
        return 1;
    }

    if (!bus.processor.ipApplied) {
        std::cerr << "[Server] Missing ip command; TCP bus was never initialized.\n";
        runtime.run(false);
        busThread.shutdown();
        return 1;
    }

    std::cout << "[Log] Writing " << logPath << "\n";
    std::cout << "[Server] Running. Press Enter to shutdown..." << "\n";
    std::cin.get();

    runtime.run(false);
    busThread.shutdown();
    return 0;
}
