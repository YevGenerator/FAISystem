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
#include "../Core/logger/Logger.hpp"
#include "../Core/nodes/NodeTable.hpp"
#include "../Core/sensor/SensorMaster.hpp"
#include "WorkerPool.hpp"
#include "ZMRouter.hpp"
#include "../Core/types/overload.hpp"

#include "../ZMCore/ZMBus.hpp"
#include "../ZMCore/ThreadRunner.hpp"
#include "../ZMCore/queues/ZMQueueList.hpp"
#include "events/EventCounter.hpp"
#include "nodes/Router.hpp"

namespace NodeSystem::ZMCore {

    template<bool IsServer>
    class RpController {
    public:
        using ForwardPushQueue = std::conditional_t<IsServer,
            Queues::ZMBusServerForwardPush,
            Queues::ZMBusClientForwardPush>;

        using Header = Core::Events::EventHeader;

        RpController()
            : zmq_context(1),
              sensorMaster(zmq_context),
              workerMaster(zmq_context, nodeStore),
              router(zmq_context, nodeStore, *this),
              forwardPush(zmq_context)
        {}

        bool loadAndExecute(const std::string &binFilePath) {
            std::string cleanPath = binFilePath;
            while (!cleanPath.empty() && (std::isspace(cleanPath.back()) || cleanPath.back() == '\r' || cleanPath.back() == '\n')) {
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

        template<typename P>
        void executeNetworkCommand(const Core::Commands::CommandList::NetworkPacket<P> &packet) {
            std::visit(Core::overload{
                           [&](std::monostate) {},
                           [&](const auto &cmd) { this->process(cmd, packet.eventHeader); },
                       }, packet.data);
        }

        void executeCommand(const Core::Commands::CommandList::Variant &cmd) {
            std::visit(Core::overload{
                           [&](std::monostate) {},
                           [&](const auto &comm) { this->process(comm, Core::Events::EventCounter::newEvent(0)); }
                       }, cmd);
        }

        template<typename T>
        void pushToForward(const T &command, Core::ID targetDevice = 0) {
            // Ініціалізуємо чергу при першому виклику (це безпечно, якщо викликається з одного потоку)
            if (!forwardPushInited) {
                forwardPush.init();
                forwardPushInited = true;
            }

            if constexpr (IsServer) {
                forwardPush.push(targetDevice, command);
            } else {
                forwardPush.push(command);
            }
        }

        void process(const std::monostate &cmd, const Header& header) {}

        void process(const Core::Commands::CmdDevice &cmd, const Header& header) {
            Core::DeviceConfig::deviceId = cmd.deviceId;
            //LoggerBin.log(logs::LogSetDeviceId{cmd.deviceId}, header);
        }

        void process(const Core::Commands::CmdWorkers &cmd, const Header& header) {
            this->workerMaster.initWorkers(cmd.workerAmount);
            //LoggerBin.log(logs::LogSetWorkers{cmd.workerAmount}, header);
        }

        void process(const Core::Commands::CmdRun &cmd, const Header& header) {
            if constexpr (IsServer) {
                if (cmd.device != DeviceConfig::deviceId) {
                    this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.device);
                    LoggerBin.log(logs::LogRun{cmd.device, cmd.toRun, true}, header);
                    return;
                }
            }
            if (cmd.toRun) {
                this->router.launch();
                this->workerMaster.startAll();
                this->sensorMaster.launch();
                LoggerBin.log(logs::LogRun{cmd.device, cmd.toRun, false}, header);
                return;
            }
            this->sensorMaster.shutdown();
            this->workerMaster.shutdown();
            this->router.shutdown();
            LoggerBin.log(logs::LogRun{cmd.device, cmd.toRun, false}, header);
        }

        void process(const ConfigCmdIp &cmd, const Header& header) {
            DeviceConfig::serverAddress.host = cmd.ipData.host;
            DeviceConfig::serverAddress.port = cmd.ipData.port;

            // Динамічно створюємо та запускаємо автобус
            if constexpr (IsServer) {
                busTask.emplace(zmq_context, this, DeviceConfig::serverAddress);
            } else {
                DeviceConfig config;
                config.serverAddress = DeviceConfig::serverAddress;
                config.deviceId = DeviceConfig::deviceId;
                busTask.emplace(zmq_context, this, config);
            }

            busThread.emplace(*busTask);
            busThread->launch();

            LoggerBin.log(logs::LogSetIp{cmd.ipData}, header);
        }

        void process(const ConfigCmdRead &cmd, const Header& header) {
            this->loadAndExecute(cmd.filename);
        }

        void process(const NodeResultMessage &cmd, const Header &header) {
            NodeResult packet;
            packet.eventHeader = header;
            packet.data = cmd;
            this->router.pushResultMessage(packet);
        }

        void process(const NodeProcessMessage &cmd, const Header &header) {
            // this->router.pushResultMessage(CommandList::CreateNetworkPacket(cmd, tag));
        }

        void process(const ConfigCmdSensorCreate &cmd, const Header &header) {
            if constexpr (IsServer) {
                if (cmd.device != DeviceConfig::deviceId) {
                    this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.device);
                    LoggerBin.log(logs::LogCreateSensor{cmd.device, cmd.id_val, true}, header);
                    return;
                }
            }
            this->sensorMaster.addOrUpdateSensor(cmd.id_val, cmd.period, cmd.valueToEmit);
            LoggerBin.log(logs::LogCreateSensor{cmd.device, cmd.id_val, false}, header);
        }

        void process(const ConfigCmdNodeCreate &cmd, const Header& header) {
            if constexpr (IsServer) {
                if (cmd.device != DeviceConfig::deviceId) {
                    this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.device);
                    LoggerBin.log(logs::LogCreateNode{
                        cmd.device, cmd.id_lvl, cmd.id_val, cmd.algo, cmd.output, cmd.inputs_count, true
                    }, header);
                    return;
                }
            }
            this->nodeStore.createNode(cmd);
            LoggerBin.log(logs::LogCreateNode{
                cmd.device, cmd.id_lvl, cmd.id_val, cmd.algo, cmd.output, cmd.inputs_count, false
            }, header);
        }

        void process(const ConfigCmdNodeAddInput &cmd, const Header& header) {
            if constexpr (IsServer) {
                if (cmd.device != DeviceConfig::deviceId) {
                    this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.device);
                    LoggerBin.log(logs::LogNodeAddInput{
                        cmd.device, cmd.id_lvl, cmd.id_val, cmd.input_index, cmd.input_data, true
                    }, header);
                    return;
                }
            }
            this->nodeStore.addInputToNode({cmd.id_lvl, cmd.id_val}, cmd.input_data, cmd.input_index);
            LoggerBin.log(logs::LogNodeAddInput{
                cmd.device, cmd.id_lvl, cmd.id_val, cmd.input_index, cmd.input_data, false
            }, header);
        }

        void process(const ConfigCmdBindSingle &cmd, const Header& header) {
            if constexpr (IsServer) {
                if (cmd.from.device != DeviceConfig::deviceId) {
                    this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.from.device);
                    LoggerBin.log(logs::LogSingleBind{cmd.from, cmd.deviceTo, true}, header);
                    return;
                }
                if (cmd.deviceTo != DeviceConfig::deviceId) {
                    this->nodeStore.bind({cmd.from.id_lvl, cmd.from.id_val}, ExternalAddress{cmd.deviceTo});
                    LoggerBin.log(logs::LogSingleBind{cmd.from, cmd.deviceTo, false}, header);
                    return;
                }
            }
            this->nodeStore.bind({cmd.from.id_lvl, cmd.from.id_val});
            LoggerBin.log(logs::LogSingleBind{cmd.from, cmd.deviceTo, false}, header);
        }

        void process(const ConfigCmdBindDouble &cmd, const Header& header) {
            if constexpr (IsServer) {
                if (cmd.from.device != DeviceConfig::deviceId) {
                    if (cmd.to.device == cmd.from.device) {
                        this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.from.device);
                        LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, true}, header);
                        return;
                    }
                    if (cmd.to.device == DeviceConfig::deviceId) {
                        const auto singleCmd = ConfigCmdBindSingle{cmd.from};
                        this->pushToForward(CommandList::CreateNetworkPacket(singleCmd, header), cmd.from.device);
                        LoggerBin.log(logs::LogSingleBind{cmd.from, DeviceConfig::deviceId, true}, header);
                    } else {
                        const auto singleFrom = ConfigCmdBindSingle{cmd.from};
                        this->pushToForward(CommandList::CreateNetworkPacket(singleFrom, header), cmd.from.device);
                        LoggerBin.log(logs::LogSingleBind{cmd.from, DeviceConfig::deviceId, true}, header);

                        this->nodeStore.bind({cmd.from.id_lvl, cmd.from.id_val}, ExternalAddress{cmd.to.device});
                        LoggerBin.log(logs::LogSingleBind{cmd.from, cmd.to.device, false}, header);

                        this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.to.device);
                        LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, true}, header);
                        return;
                    }
                } else {
                    if (cmd.to.device != DeviceConfig::deviceId) {
                        this->nodeStore.bind({cmd.from.id_lvl, cmd.from.id_val}, ExternalAddress{cmd.to.device});
                        LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, false}, header);

                        this->pushToForward(CommandList::CreateNetworkPacket(cmd, header), cmd.to.device);
                        Core::LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, true}, header);
                        return;
                    }
                }
            }
            this->nodeStore.bind(
                {cmd.from.id_lvl, cmd.from.id_val},
                Core::Nodes::InternalAddress{{cmd.to.id_lvl, cmd.to.id_val}, cmd.to.in_index});
            //LoggerBin.log(logs::LogDoubleBind{cmd.from, cmd.to, false}, header);
        }
        void launchBlocking() {
            run({});
        }

        void run(std::stop_token token) {
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
                if (line.empty()) { continue;
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

    protected:
        static std::string formatHost(std::uint32_t host) {
            return std::to_string(host & 0xFF) + "." +
                   std::to_string((host >> 8) & 0xFF) + "." +
                   std::to_string((host >> 16) & 0xFF) + "." +
                   std::to_string((host >> 24) & 0xFF);
        }

    private:
        zmq::context_t zmq_context;
        Core::Nodes::NodeTable nodeStore;
        Core::Sensors::SensorMaster<> sensorMaster{};
        WorkerPool workerMaster;
        ZMRouter router;

        bool forwardPushInited = false;
        ForwardPushQueue forwardPush; // Черга для відправки у ZMBus

        // ZMBus запускається динамічно при отриманні команди ConfigCmdIp
        std::optional<ZMCore::ZMBus<IsServer>> busTask;
        std::optional<ZMCore::ThreadRunner<ZMCore::ZMBus<IsServer>>> busThread;
    };

} // namespace NodeSystem::Core