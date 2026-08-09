#pragma once

#include <stop_token>
#include <array>
#include <zmq.hpp>

#include "queues/ZMQueueList.hpp"
#include "commands/CommandList.hpp"

namespace NodeSystem::ZMCore {
    inline auto readNetworkPacket(const zmq::message_t &payload) {
        const auto *data = payload.data<const char>();
        const auto messageType = static_cast<Core::types::Byte>(data[0]);
        constexpr auto networkRead = Core::Commands::CommandList::handlersNetwork();
        return networkRead[messageType](data);
    }

    template<bool IsServer>
    class ZMBus;

    template<>
    class ZMBus<true> {
    public:
        template<typename ControllerT>
        ZMBus(zmq::context_t &context, ControllerT *controller, const Core::Commands::KgIP &serverAddress)
            : tcpQueue(context), forwardQueue(context), controller(controller), serverAddress(serverAddress) {
        }

        void run(const std::stop_token &stopToken) {
            tcpQueue.init(serverAddress);
            forwardQueue.init();

            std::array<zmq::pollitem_t, 2> pollItems = {
                zmq::pollitem_t{tcpQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
                zmq::pollitem_t{forwardQueue.socket.handle(), 0, ZMQ_POLLIN, 0}
            };

            while (!stopToken.stop_requested()) {
                try {
                    zmq::poll(pollItems.data(), pollItems.size(), std::chrono::milliseconds(50));
                    processTcp(pollItems[0]);
                    processForward(pollItems[1]);
                } catch (const zmq::error_t &) {
                }
            }
        }

    private:
        void processTcp(const zmq::pollitem_t &item) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto msg = tcpQueue.pull()) {
                auto &[clientId, payload] = *msg;
                if (!payload.empty()) {
                    auto command = readNetworkPacket(payload);
                    controller->executeNetworkCommand(command);
                }
            }
        }

        void processForward(const zmq::pollitem_t &item) {
            if (!(item.revents & ZMQ_POLLIN)) return;

            if (auto msg = forwardQueue.pull()) {
                auto &[targetId, payload] = *msg;
                try {
                    // Одразу відправляємо в TCP. Якщо клієнт недоступний — дропаємо
                    tcpQueue.push(targetId, payload);
                } catch (const zmq::error_t &) {
                }
            }
        }

        Queues::ZMBusServerTcp tcpQueue{};
        Queues::ZMBusClientForward forwardQueue;
        void *controller;
        Core::Commands::KgIP serverAddress;
    };

    template<>
    class ZMBus<false> {
    public:
        template<typename ControllerT>
        ZMBus(zmq::context_t &context, ControllerT *controller, const Core::DeviceConfig &config)
            : tcpQueue(context), forwardQueue(context), controller(controller), config(config) {
        }

        void run(const std::stop_token &stopToken) {
            tcpQueue.init(config);
            forwardQueue.init();

            std::array pollItems = {
                zmq::pollitem_t{tcpQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
                zmq::pollitem_t{forwardQueue.socket.handle(), 0, ZMQ_POLLIN, 0},
            };

            while (!stopToken.stop_requested()) {
                try {
                    zmq::poll(pollItems.data(), pollItems.size(), std::chrono::milliseconds(50));
                    processTcp(pollItems[0]);
                    processForward(pollItems[1]);
                } catch (const zmq::error_t &) {
                }
            }
        }

    private:
        void processTcp(const zmq::pollitem_t &item) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto payload = tcpQueue.pull()) {
                auto command = readNetworkPacket(*payload);
                controller->executeNetworkCommand(command);
            }
        }

        void processForward(const zmq::pollitem_t &item) {
            if (!(item.revents & ZMQ_POLLIN)) {
                return;
            }

            if (auto payload = forwardQueue.pull()) {
                try {
                    tcpQueue.push(*payload);
                } catch (const zmq::error_t &) {
                }
            }
        }

        Queues::ZMBusClientTcp tcpQueue;
        Queues::ZMBusClientForward forwardQueue;
        void *controller;
        Core::DeviceConfig config;
    };
} // namespace NodeSystem::ZMCore
