#pragma once

#include <stop_token>
#include <array>
#include <zmq.hpp>

#include "CmdProcessor.hpp"
#include "queues/ZMQueueList.hpp"
#include "commands/CommandList.hpp"
#include "queues/ZMQueueList.hpp"

namespace NodeSystem::ZMCore {
    inline auto readNetworkPacket(const zmq::message_t &payload) {
        const auto *data = payload.data<const char>();
        const auto messageType = static_cast<Core::types::Byte>(data[0]);
        constexpr auto networkRead = Core::Commands::CommandList::handlersNetwork();
        return networkRead[messageType](data);
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
                    auto command = readNetworkPacket(payload);
                    processor.executePacket(command);
                }
            }
        }

        void processTcp(const zmq::pollitem_t &item) requires (!IsServer) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = tcpQueue.pull()) {
                auto command = readNetworkPacket(*msg);
                processor.executePacket(command);
            }
        }

        void processForward(const zmq::pollitem_t &item) requires IsServer {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = forwardQueue.pull()) {
                auto &[targetId, payload] = *msg;
                try {
                    tcpQueue.push(targetId, payload);
                } catch (const zmq::error_t &) {
                }
            }
        }

        void processForward(const zmq::pollitem_t &item) requires (!IsServer) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = forwardQueue.pull()) {
                try {
                    tcpQueue.push(*msg);
                } catch (const zmq::error_t &) {
                }
            }
        }
    };
} // namespace NodeSystem::ZMCore
