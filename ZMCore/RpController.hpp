#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stop_token>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include <zmq.hpp>

#include "../strings/CommandParser.hpp"
#include "ConfigReader.hpp"
#include "../coreі/logger/Logger.hpp"
#include "../coreі/nodes/NodeTable.hpp"
#include "../coreі/sensor/SensorMaster.hpp"
#include "WorkerPool.hpp"
#include "ZMRouter.hpp"
#include "ZMSensors.hpp"
#include "../coreі/types/overload.hpp"

#include "../ZMCore/ZMBus.hpp"
#include "../ZMCore/ThreadRunner.hpp"
#include "../ZMCore/queues/ZMQueueList.hpp"
#include "events/EventCounter.hpp"
#include "nodes/Router.hpp"

namespace NodeSystem::ZMCore {
    template<bool IsServer>
    class RpController {
    public:
        using Header = Core::Events::EventHeader;
        using SensorThread = ThreadRunner<ZMSensors>;
        using RouterThread = ThreadRunner<ZMRouter>;
        using BusThread = ThreadRunner<ZMBus<IsServer> >;

        RpController()
            : zmq_context(1),
              sensorThread(zmq_context),
              workerMaster(zmq_context),
              router(zmq_context),
              networkBus(zmq_context) {
        }


        bool loadAndExecute(const std::string &binFilePath) {
            std::string cleanPath = binFilePath;
            while (!cleanPath.empty() && (std::isspace(cleanPath.back()) || cleanPath.back() == '\r' || cleanPath.back()
                                          == '\n')) {
                cleanPath.pop_back();
            }
            while (!cleanPath.empty() && std::isspace(cleanPath.front())) {
                cleanPath.erase(0, 1);
            }
            if (cleanPath.size() >= 2 && cleanPath.front() == '"' && cleanPath.back() == '"') {
                cleanPath = cleanPath.substr(1, cleanPath.length() - 2);
            }
            std::filesystem::path filePath;

#ifdef _WIN32
            if (!cleanPath.empty()) {
                UINT cp = GetConsoleCP();
                int size_needed = MultiByteToWideChar(cp, 0, cleanPath.c_str(), (int) cleanPath.size(), NULL, 0);
                std::wstring wstr(size_needed, 0);
                MultiByteToWideChar(cp, 0, cleanPath.c_str(), (int) cleanPath.size(), &wstr[0], size_needed);
                filePath = std::filesystem::path(wstr);
            }
#else
            filePath = std::filesystem::path(cleanPath);
#endif
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);

            if (!file) {
                std::cerr << "[Loader] Failed to open file: " << cleanPath << "\n";
                return false;
            }

            const auto size = file.tellg();
            if (size < sizeof(Core::FileHeader)) return false;

            file.seekg(0);
            std::vector<char> buffer(size);
            file.read(buffer.data(), size);

            auto *header = reinterpret_cast<Core::FileHeader *>(buffer.data());
            if (header->magic != Core::MAGIC_BYTES || header->version != Core::FORMAT_VERSION) {
                std::cerr << "[Loader] Invalid format or version mismatch!\n";
                return false;
            }

            const char *ptr = buffer.data() + sizeof(Core::FileHeader);
            std::size_t available = static_cast<std::size_t>(size) - sizeof(Core::FileHeader);

            std::cout << "[Loader] Starting system initialization...\n";

            while (available > 0) {
                auto variantOpt = Core::Commands::CommandList::nextFromFile(ptr, available);
                if (!variantOpt) {
                    std::cerr << "[Loader] Stream parsing error!\n";
                    return false;
                }
                executeCommand(*variantOpt);
            }

            std::cout << "[Loader] System initialization complete.\n";
            return true;
        }

        void launchBlocking() {
            cli({});
        }

        void initBus() {
            CmdProcessorContext<IsServer> context;
            context.run = RunDelegate::create<&RpController::run>(this);
            context.routeTable = this->router.task.coreRouter.routes;
            context.nodeTable = this->workerMaster.nodeStore;
            context.sensors = this->sensorThread.task.sensors;
            this->networkBus.task.init(context);
        }

        void run(bool toRun) {
            if (toRun) {
                this->router.launch();
                this->workerMaster.startAll();
                this->sensorThread.launch();
                return;
            }
            this->sensorThread.shutdown();
            this->workerMaster.shutdown();
            this->router.shutdown();
        }

        void reconnect() {
            this->networkBus.shutdown();
            this->networkBus.launch();
        }

        void cli(std::stop_token token) {
            std::string line;
            std::map<std::string, std::string> emptyEnv;
            Core::types::ID currentDevice = 0;

            std::cout << "==========================================\n";
            std::cout << "   NodeSystem Interactive Shell v1.1\n";
            std::cout << "   Powered by Algoholic Registry\n";
            std::cout << "==========================================\n";

            while (!token.stop_requested()) {
                std::cout << "rpsh> ";
                if (!std::getline(std::cin, line)) break;

                line = Core::strings::CommandParser::trim(line);
                if (line.empty()) {
                    continue;
                }
                if (line == "exit" || line == "quit") break;

                auto commands = strings::CommandParser::parseCommand(line, emptyEnv, currentDevice);

                if (commands.empty()) {
                    std::cout << "[Shell] Warning: No commands parsed or unknown syntax.\n";
                    continue;
                }

                for (const auto &variantCmd: commands) {
                    std::visit(Core::overload{
                                   [&](const auto &cmd) { this->process(cmd, Core::Events::EventCounter::newEvent(0)); }
                               }, variantCmd);
                }
            }
        }

    private:
        zmq::context_t zmq_context;
        SensorThread sensorThread;
        WorkerPool workerMaster;
        RouterThread router;
        BusThread networkBus;
    };
} // namespace NodeSystem::ZMCore
