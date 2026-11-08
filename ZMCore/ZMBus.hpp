#pragma once

#include <stop_token>
#include <array>
#include <optional>
#include "logger/Logger.hpp"
#include <zmq.hpp>

#include "CmdProcessor.hpp"
#include "queues/ZMQueueList.hpp"
#include "commands/CommandList.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore {
    inline auto readNetworkPacket(const zmq::message_t &payload)
        -> std::optional<Core::Commands::NetworkPacket<Core::Commands::CommandList::Variant>> {
        if (payload.empty()) return std::nullopt;
        const auto* data = payload.data<const char>();
        auto messageType = static_cast<Core::types::Byte>(static_cast<unsigned char>(data[0]));
        constexpr auto readers = Core::Commands::CommandList::handlersNetwork();
        constexpr auto sizes = Core::Commands::CommandList::networkPacketSizes();
        if (messageType >= readers.size() || payload.size() != sizes[messageType]) return std::nullopt;
        return readers[messageType](data);
    }

    template<bool IsServer>
    class ZMBus {
    public:
        std::atomic<bool> needs_reconnect{false};
        Core::DeviceConfig config;
        std::conditional_t<IsServer, Queues::ZMBusServerTcp, Queues::ZMBusClientTcp> tcpQueue;
        std::conditional_t<IsServer, Queues::ZMBusServerForward, Queues::ZMBusClientForward> forwardQueue;
        Queues::ZMRouterPush routerQueue;

    public:
        CmdProcessor<IsServer> processor;

        explicit ZMBus(zmq::context_t &context)
            : tcpQueue(context)
            , forwardQueue(context)
            , routerQueue(context) {
        }

        void init(CmdProcessorContext<IsServer> context) {
            context.deviceConfig = &config;
            context.tcpQueue = &tcpQueue;
            context.routerQueue = &routerQueue;
            context.need_reconnect = &needs_reconnect;
            this->processor.init(context);
        }

        void run(const std::stop_token &stopToken) {
            tcpQueue.init(config);
            forwardQueue.init();
            routerQueue.init();

            std::array pollItems = {
                zmq::pollitem_t{tcpQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
                zmq::pollitem_t{forwardQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
            };

            while (!stopToken.stop_requested()) {
                if (needs_reconnect.load(std::memory_order_relaxed)) {
                    this->reconnectTcp();
                    pollItems[0].socket = tcpQueue.socket.handle();
                    needs_reconnect.store(false, std::memory_order_relaxed);
                }

                try {
                    zmq::poll(pollItems.data(), pollItems.size(), std::chrono::milliseconds(50));
                    processTcp(pollItems[0]);
                    processForward(pollItems[1]);
                } catch (const zmq::error_t &) {
                }
            }
        }

        void reconnectTcp() {
            tcpQueue.socket.close();
            tcpQueue.init(config);
        }

        void processTcp(const zmq::pollitem_t &item) requires IsServer {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = tcpQueue.pull()) {
                auto &[clientId, payload] = *msg;
                if (!payload.empty()) {
                    if (auto command = readNetworkPacket(payload)) {
                        Core::LoggerBin.log(Core::types::logs::LogReceivedTcpCommand{
                            clientId, config.deviceId
                        }, Core::LoggerBin.newEvent(command->eventHeader));
                        processor.executePacket(*command);
                    }
                }
            }
        }

        void processTcp(const zmq::pollitem_t &item) requires (!IsServer) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = tcpQueue.pull()) {
                if (auto command = readNetworkPacket(*msg)) {
                    Core::LoggerBin.log(Core::types::logs::LogReceivedTcpCommand{
                        0, config.deviceId
                    }, Core::LoggerBin.newEvent(command->eventHeader));
                    processor.executePacket(*command);
                }
            }
        }

        void processForward(const zmq::pollitem_t &item) requires IsServer {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto packet = forwardQueue.pull()) {
                try {
                    Core::LoggerBin.log(Core::types::logs::LogSendTcpCommand{
                        config.deviceId, packet->data.deviceId
                    }, Core::LoggerBin.newEvent(packet->eventHeader));
                    tcpQueue.push(packet->data.deviceId, *packet);
                } catch (const zmq::error_t &) {
                }
            }
        }

        void processForward(const zmq::pollitem_t &item) requires (!IsServer) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto packet = forwardQueue.pull()) {
                try {
                    Core::LoggerBin.log(Core::types::logs::LogSendTcpCommand{
                        config.deviceId, packet->data.deviceId
                    }, Core::LoggerBin.newEvent(packet->eventHeader));
                    tcpQueue.push(*packet);
                } catch (const zmq::error_t &) {
                }
            }
        }
    };
} // namespace NodeSystem::ZMCore
