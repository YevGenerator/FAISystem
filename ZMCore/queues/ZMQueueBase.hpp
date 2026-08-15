#pragma once
#include <zmq.hpp>

#include "policies/connect_policy.hpp"
#include "policies/protocol_policy.hpp"

namespace NodeSystem::ZMCore::Queues {
    template<typename T>
    concept IsZMQueue = requires(T t)
    {
        { t.init() } -> std::same_as<void>;
        { t.socket } -> std::same_as<zmq::socket_t &>;
    };

    template<typename ConnectPolicy, Policies::IsProtocolPolicy Protocol>
    struct ZMQueueBase {
        using target_t = Protocol::target_t;

        ZMQueueBase(zmq::context_t &context) : context(context) {
        }

        template<typename... Args>
        void init(Args &&... args) requires Policies::IsConnectPolicy<ConnectPolicy, Args...> {
            ConnectPolicy::init(socket, context, std::forward<Args>(args)...);
        }

        template<typename T>
        auto push(const Core::ID deviceId, T &&arg) requires Policies::IsPushTCPServerProtocolPolicy<Protocol> {
            return Protocol::push(this->socket, std::forward<T>(arg), deviceId);
        }

        template<typename T>
        auto push(T &&arg) requires Policies::IsPushProtocolPolicy<Protocol> {
            return Protocol::push(this->socket, std::forward<T>(arg));
        }


        auto pull() requires Policies::IsPullProtocolPolicy<Protocol> {
            return Protocol::pull(this->socket);
        }

        zmq::socket_t socket;
        zmq::context_t &context;
    };
} // namespace NodeSystem::ZMCore::Queues
