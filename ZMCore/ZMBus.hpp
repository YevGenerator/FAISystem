#pragma once

#include <stop_token>
#include <condition_variable>
#include <mutex>
#include <exception>
#include <stdexcept>
#include <set>
#include <map>
#include <deque>
#include <chrono>
#include "OutboundQueue.hpp"
#include <array>
#include <optional>
#include <iostream>
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
        OutboundQueue outbound;
        Core::DeviceConfig config;
        std::conditional_t<IsServer, Queues::ZMBusServerTcp, Queues::ZMBusClientTcp> tcpQueue;
        std::conditional_t<IsServer, Queues::ZMBusServerForward, Queues::ZMBusClientForward> forwardQueue;
        Queues::ZMRouterPush routerQueue;

    public:
        CmdProcessor<IsServer> processor;

    private:
        std::mutex readyMutex;
        std::condition_variable readyCv;
        bool startupComplete = false;
        std::exception_ptr startupError;
        std::map<Core::types::ID, std::deque<OutboundMessage>> pending;
        std::set<Core::types::ID> connectedClients;

    public:
        void awaitReady() {
            std::unique_lock lock(readyMutex);
            if (!readyCv.wait_for(lock, std::chrono::seconds(10), [&] { return startupComplete; }))
                throw std::runtime_error("Timed out initializing ZMBus");
            if (startupError) std::rethrow_exception(startupError);
        }


        explicit ZMBus(zmq::context_t &context)
            : tcpQueue(context)
            , forwardQueue(context)
            , routerQueue(context) {
        }

        void init(CmdProcessorContext<IsServer> context) {
            context.deviceConfig = &config;
            context.tcpQueue = &tcpQueue;
            context.routerQueue = &routerQueue;
            context.outbound = &outbound;
            this->processor.init(context);
        }

        void run(const std::stop_token &stopToken) {
            try {
                // Socket initialization and all subsequent socket operations belong
                // to this thread, never to the configuration-reader thread.
                tcpQueue.init(config);
                forwardQueue.init();
                routerQueue.init();
                {
                    std::scoped_lock lock(readyMutex);
                    startupComplete = true;
                }
                readyCv.notify_all();
            } catch (...) {
                {
                    std::scoped_lock lock(readyMutex);
                    startupError = std::current_exception();
                    startupComplete = true;
                }
                readyCv.notify_all();
                return;
            }

            std::array pollItems = {
                zmq::pollitem_t{tcpQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
                zmq::pollitem_t{forwardQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
            };

            while (!stopToken.stop_requested()) {
                try {
                    zmq::poll(pollItems.data(), pollItems.size(), std::chrono::milliseconds(25));
                    processTcp(pollItems[0]);
                    processForward(pollItems[1]);
                    dispatchOutbound();
                } catch (const zmq::error_t &error) {
                    std::cerr << "[ZMBus] " << error.what() << '\\n';
                }
            }
        }

        // FIFO per destination; server waits for a DEALER probe before sending.
        void dispatchOutbound() {
            for (auto& message : outbound.drain())
                pending[message.deviceId].push_back(std::move(message));
            for (auto& [device, queue] : pending) {
                if constexpr (IsServer) {
                    if (!connectedClients.contains(device)) continue;
                }
                while (!queue.empty()) {
                    const auto& msg = queue.front();
                    try {
                        if constexpr (IsServer) {
                            tcpQueue.socket.send(zmq::buffer(&device, sizeof(device)), zmq::send_flags::sndmore);
                        }
                        tcpQueue.socket.send(zmq::buffer(msg.bytes.data(), msg.bytes.size()), zmq::send_flags::none);
                        queue.pop_front();
                    } catch (const zmq::error_t& error) {
                        if constexpr (IsServer) connectedClients.erase(device);
                        std::cerr << "[ZMBus] Send failed: " << error.what() << '\\n';
                        break;
                    }
                }
            }
        }

        void processTcp(const zmq::pollitem_t &item) requires IsServer {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = tcpQueue.pull()) {
                auto &[clientId, payload] = *msg;
                connectedClients.insert(clientId);
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
                    outbound.push(packet->data.deviceId, *packet);
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
                    outbound.push(packet->data.deviceId, *packet);
                } catch (const zmq::error_t &) {
                }
            }
        }
    };
} // namespace NodeSystem::ZMCore
