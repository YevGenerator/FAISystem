#pragma once
#include <zmq.hpp>

#include "commands/CommandList.hpp"

namespace NodeSystem::ZMCore::Queues::Policies {
    template<typename T>
    concept IsPullProtocolPolicy = requires(zmq::socket_ref socket)
    {
        typename T::target_t;
        T::pull(socket);
    };

    template<typename T>
    concept IsPushProtocolPolicy = requires(zmq::socket_ref socket, const typename T::target_t &target)
    {
        typename T::target_t;
        T::push(socket, target);
    };

    template<typename T>
    concept IsProtocolPolicy = IsPullProtocolPolicy<T> || IsPushProtocolPolicy<T>;

    template<typename TTarget>
    struct PullerBase {
        using target_t = TTarget;

        PullerBase(zmq::socket_ref socket) : socket(socket) {
        }

        zmq::socket_ref socket;

        auto pull() const -> std::optional<target_t> {
            return this->pull(socket);
        }

        static auto pull(zmq::socket_ref socket) -> std::optional<target_t> {
            target_t packet;
            auto response = socket.recv(zmq::buffer(&packet, sizeof(packet)));
            if (!response) {
                return std::nullopt;
            }
            return packet;
        }
    };

    template<typename TTarget>
    struct PusherBase {
        using target_t = TTarget;

        PusherBase(zmq::socket_ref socket) : socket(socket) {
        }

        zmq::socket_ref socket;

        void push(const target_t &target) const {
            this->push(socket, target);
        }

        static void push(zmq::socket_ref socket, const target_t &target) {
            socket.send(zmq::buffer(&target, sizeof(target)));
        }
    };


    struct ServerTcpProtocolPolicy {
        using target_t = std::pair<Core::ID, zmq::message_t>;

        static auto pull(zmq::socket_ref socket) -> std::optional<target_t> {
            zmq::message_t identity;
            if (!socket.recv(identity, zmq::recv_flags::none)) {
                return std::nullopt;
            }

            Core::ID clientId{};
            std::memcpy(&clientId, identity.data(), std::min(sizeof(clientId), identity.size()));

            zmq::message_t payload;
            if (!socket.recv(payload, zmq::recv_flags::none)) {
                return std::nullopt;
            }

            return target_t{clientId, std::move(payload)};
        }

        static void push(zmq::socket_ref socket, Core::ID targetId, zmq::message_t &payload) {
            zmq::message_t payloadCopy;
            payloadCopy.copy(payload);
            socket.send(zmq::buffer(&targetId, sizeof(targetId)), zmq::send_flags::sndmore);
            socket.send(payloadCopy, zmq::send_flags::none);
        }
    };

    struct ClientTcpProtocolPolicy {
        using target_t = zmq::message_t;

        static auto pull(zmq::socket_ref socket) -> std::optional<target_t> {
            zmq::message_t payload;
            if (!socket.recv(payload, zmq::recv_flags::none) || payload.empty()) {
                return std::nullopt;
            }
            return payload;
        }

        static void push(zmq::socket_ref socket, zmq::message_t &payload) {
            zmq::message_t payloadCopy;
            payloadCopy.copy(payload);
            socket.send(payloadCopy, zmq::send_flags::none);
        }
    };

    static_assert(IsProtocolPolicy<ServerTcpProtocolPolicy>);
    static_assert(IsProtocolPolicy<ClientTcpProtocolPolicy>);
} // namespace NodeSystem::ZMCore::Queues::Policies
